/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */

#include "IHsl.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IHsl)
#endif

namespace hku {

IHsl::IHsl() : IndicatorImp("HSL", 1) {}

IHsl::~IHsl() {}

IHsl::IHsl(const KData& k) : IndicatorImp("HSL", 1) {
    setParam<KData>("kdata", k);
    IHsl::_calculate(Indicator());
}

void IHsl::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    // 先将 discard 设为全部，后续更新
    m_discard = total;

    Stock stock = k.getStock();
    auto* kdata = k.data();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();

    StockWeightList sw_list = stock.getWeight(Datetime::min(), lastdate + Days(1));
    HKU_IF_RETURN(sw_list.empty(), void());

    // 寻找第一个流通盘不为0的权息
    price_t pre_free_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->freeCount() > 0) {
            pre_free_count = sw_iter->freeCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }

    // 没有流通盘相关权息数据, 或者该权息日期大于最后一根K线日期, 直接返回
    HKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

    auto* dst = this->data();
    size_t pos = 0;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (free_count <= 0.0) {
            continue;  // 忽略流通盘为0的权息
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            if (kdata[pos].datetime >= pre_sw_date) {
                // transCount 单位为手数，流通盘单位为万股
                dst[pos] = kdata[pos].transCount / pre_free_count * 0.01;
            }
            pos++;
        }

        pre_free_count = free_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        dst[pos] = kdata[pos].transCount / pre_free_count * 0.01;
    }

    // 更新 discard
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API HSL() {
    return make_shared<IHsl>()->calculate();
}

Indicator HKU_API HSL(const KData& k) {
    return Indicator(make_shared<IHsl>(k));
}

} /* namespace hku */
