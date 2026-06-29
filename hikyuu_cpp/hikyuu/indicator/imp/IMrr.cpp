/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMrr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMrr)
#endif

namespace hku {

IMrr::IMrr() : IndicatorImp("MRR", 1) {
    setParam<int>("n", 0);
}

IMrr::~IMrr() {}

void IMrr::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMrr::_calculate(const Indicator& ind) {
    m_discard = ind.discard();
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard >= total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t n = static_cast<size_t>(getParam<int>("n"));
    if (n == 0 || n > total - m_discard) {
        n = total - m_discard;
    }

    if (n == 1) {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = 0.0;
        }
        return;
    }

    if (n == total - m_discard) {
        value_t pre_min = src[m_discard];
        value_t max_rr = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] < pre_min || pre_min == 0.) {
                pre_min = src[i];
            }
            value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
            if (rr > max_rr) {
                max_rr = rr;
            }
            dst[i] = max_rr * 100.0;
        }
        return;
    }

    value_t pre_min = src[m_discard];
    value_t max_rr = 0.0;
    for (size_t i = m_discard; i < m_discard + n; ++i) {
        if (src[i] < pre_min || pre_min == 0.) {
            pre_min = src[i];
        }
        value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
        if (rr > max_rr) {
            max_rr = rr;
        }
        dst[i] = max_rr * 100.0;
    }

    if (m_discard + n < total) {
        _increment_calculate(ind, m_discard + n);
    }
}

bool IMrr::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMrr::min_increment_start() const {
    return getParam<int>("n");
}

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // 标准最大盈利比率语义: rr_j = src[j] / run_min_j - 1,
    // 其中 run_min_j = min(src[window_left..j]) 为到 j 为止的累计最小值。
    // 原实现错误地用窗口全局 min 作为所有点的盈利基准, 当窗口最低点出现在
    // 最高点之后时会引入未来数据(look-ahead bias), 高估盈利比率。
    // 此处放弃原 O(1) 快路径状态机(其 current_rr > window_max_rr 判断在标准
    // MRR 语义下原理性不成立), 退化为每点 O(n) 暴力扫描, 用 run_min 基准保证
    // 正确性。与 IMdd 修复对称。
    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_price = src[i];
        if (std::isnan(current_price) || current_price <= 0.0) {
            // 无效点不写 dst[i], 保持原值, 与原语义一致
            continue;
        }

        size_t window_left = i + 1 - n;
        Indicator::value_t run_min = 0.0;  // 首个有效点赋初值, 避免 NaN 污染比较
        Indicator::value_t window_max_rr = 0.0;
        bool has_valid = false;
        for (size_t j = window_left; j <= i; ++j) {
            Indicator::value_t v = src[j];
            if (std::isnan(v) || v <= 0.0) {
                continue;
            }
            if (!has_valid) {
                run_min = v;  // 首个有效点初始化 run_min
                has_valid = true;
                continue;  // 首点 rr = v/v - 1 = 0, 跳过
            }
            if (v < run_min) {
                run_min = v;
            }
            // run_min 必然 > 0(数据约束为正), 除法安全
            Indicator::value_t rr = v / run_min - 1.0;
            if (rr > window_max_rr) {
                window_max_rr = rr;
            }
        }

        if (has_valid) {
            dst[i] = window_max_rr * 100.0;
        }
    }
}

Indicator HKU_API MRR(int n) {
    IndicatorImpPtr p = make_shared<IMrr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku