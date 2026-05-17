/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "IAdjFactor.h"
#include <algorithm>
#include <vector>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdjFactor)
#endif

namespace hku {

IAdjFactor::IAdjFactor() : IndicatorImp("ADJ_FACTOR", 1) {
    m_need_context = true;
}

IAdjFactor::~IAdjFactor() {}

// 计算对应日线复权因子(非严格上市日期，仅从 in_kdata 包含的数据开始计算)
// 注意，因为内部函数，在无权息数据清空下，直接返回空结果
// base_factor: 基准因子值，用于增量计算（默认为 1.0，表示从头开始计算）
static vector<std::pair<Datetime, Indicator::value_t>> cum_adj_factor(const KData& in_kdata,
                                                                      price_t base_factor = 1.0) {
    // 获取对应日线范围K线数据
    KData kdata = in_kdata.getKData(KQuery::DAY);

    size_t total = kdata.size();
    vector<std::pair<Datetime, Indicator::value_t>> result;
    result.reserve(total);
    HKU_IF_RETURN(total == 0, result);

    auto* krecords = kdata.data();

    // 获取所有权息数据（已按日期排序）
    StockWeightList sw_list =
      kdata.getStock().getWeight(krecords[0].datetime, krecords[total - 1].datetime + Days(1));

    if (sw_list.empty()) {
        // 没有权息数据，返回空结果
        return result;
    }

    // 双指针同步遍历：K线指针 i，权息指针 w_idx
    price_t cumulative_factor = base_factor;  // 使用基准因子作为起点
    size_t w_idx = 0;                         // 当前处理的权息事件索引

    for (size_t i = 0; i < total; ++i) {
        const Datetime& k_date = krecords[i].datetime;

        // 处理所有在当前K线日期之前或当天的权息事件
        while (w_idx < sw_list.size() && sw_list[w_idx].datetime() <= k_date) {
            const auto& weight = sw_list[w_idx];

            // 检查是否有任何权息事件
            bool has_dividend = (weight.bonus() != 0.0);
            bool has_stock_change = (weight.countAsGift() != 0.0 || weight.increasement() != 0.0 ||
                                     weight.countForSell() != 0.0 || weight.suogu() != 0.0);

            if (has_dividend || has_stock_change) {
                // 获取参考价格（权息日前一交易日收盘价）
                price_t ref_price = 0.0;
                if (i > 0) {
                    ref_price = krecords[i - 1].closePrice;
                } else {
                    ref_price = krecords[i].openPrice;
                }

                if (ref_price > 0.0) {
                    // 计算调整系数
                    price_t adjustment_ratio = 1.0;

                    if (weight.suogu() != 0.0) {
                        adjustment_ratio = 1.0 / weight.suogu();
                    } else {
                        price_t D = weight.bonus() / 10.0;
                        price_t S = weight.countAsGift() / 10.0;
                        price_t Z = weight.increasement() / 10.0;
                        price_t R = weight.countForSell() / 10.0;
                        price_t P_r = weight.priceForSell();

                        // 后复权调整系数（理论除权价公式的倒数）
                        price_t numerator = ref_price * (1.0 + S + Z + R);
                        price_t denominator = ref_price - D + P_r * R;

                        if (denominator > 0.0) {
                            adjustment_ratio = numerator / denominator;
                        }
                    }

                    // 应用调整系数（累乘）
                    if (adjustment_ratio > 0.0 && adjustment_ratio != 1.0) {
                        cumulative_factor *= adjustment_ratio;
                    }
                }
            }

            ++w_idx;  // 移动到下一个权息事件
        }

        // 添加当前日的日期和复权因子
        result.emplace_back(k_date, cumulative_factor);
    }

    return result;
}

void IAdjFactor::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 0;

    // 复用增量计算方法，start_pos = 0 表示从头开始计算
    _increment_calculate(ind, 0);
}

bool IAdjFactor::supportIncrementCalculate() const {
    return true;
}

void IAdjFactor::_increment_calculate(const Indicator& ind, size_t start_pos) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0 || start_pos >= total, void());

    auto* dst = data();
    auto* kdata = k.data();

    // 关键：复权因子必须基于日线计算
    // 优化策略：利用已计算的基准因子，只计算新增部分的权息影响

    // 1. 确定基准因子和起始日期
    price_t base_factor = 1.0;
    Datetime calc_start_date;

    if (start_pos > 0) {
        // 使用前一个位置的因子值作为基准
        base_factor = dst[start_pos - 1];
        calc_start_date = kdata[start_pos - 1].datetime.startOfDay();
    } else if (!m_old_context.empty()) {
        // 首次增量计算但有旧上下文，使用旧上下文的起始日期
        base_factor = 1.0;  // 从头开始计算
        calc_start_date = m_old_context[0].datetime.startOfDay();
    } else {
        // 真正的首次计算，从第一个K线开始
        calc_start_date = kdata[0].datetime.startOfDay();
    }

    Datetime calc_end_date = kdata[total - 1].datetime.startOfDay();

    // 2. 获取增量部分对应的K线数据（从 calc_start_date 开始）
    KData inc_kdata = k.getStock().getKData(KQueryByDate(calc_start_date, calc_end_date));

    if (inc_kdata.empty()) {
        // 没有数据，增量部分全部使用基准因子
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = base_factor;
        }
        return;
    }

    // 3. 计算增量部分的日线复权因子（传入基准因子）
    auto daily_factors = cum_adj_factor(inc_kdata, base_factor);

    if (daily_factors.empty()) {
        // 无权息数据，增量部分全部使用基准因子
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = base_factor;
        }
        return;
    }

    // 4. 将日线复权因子对齐到当前K线周期，从 start_pos 开始
    size_t d_idx = 0;
    Datetime start_date = kdata[start_pos].datetime.startOfDay();

    // 找到与 start_pos 对应日期匹配的日线因子索引
    while (d_idx < daily_factors.size() && daily_factors[d_idx].first < start_date) {
        ++d_idx;
    }

    // 5. 从 start_pos 开始填充增量部分
    price_t cumulative_factor = base_factor;

    for (size_t i = start_pos; i < total; ++i) {
        const Datetime& k_date = kdata[i].datetime.startOfDay();

        // 处理所有在当前K线日期之前或当天的日线因子
        while (d_idx < daily_factors.size() && daily_factors[d_idx].first <= k_date) {
            cumulative_factor = daily_factors[d_idx].second;
            ++d_idx;
        }

        // 设置当前K线的复权因子
        dst[i] = cumulative_factor;
    }
}

Indicator HKU_API ADJ_FACTOR() {
    return Indicator(make_shared<IAdjFactor>());
}

Indicator HKU_API ADJ_FACTOR(const KData& k) {
    auto p = make_shared<IAdjFactor>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
