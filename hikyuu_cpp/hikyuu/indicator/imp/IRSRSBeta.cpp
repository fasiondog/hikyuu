/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include "IRSRSBeta.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRSRSBeta)
#endif

namespace hku {

IRSRSBeta::IRSRSBeta() : IndicatorImp("RSRS_BETA", 1) {
    m_need_context = true;
    setParam<int>("n", 20);
}

IRSRSBeta::~IRSRSBeta() {}

void IRSRSBeta::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2);
    }
}

void IRSRSBeta::_calculate(const Indicator&) {
    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    int n = getParam<int>("n");
    if (total < static_cast<size_t>(n)) {
        m_discard = total;
        return;
    }

    auto* dst = this->data();
    m_discard = n - 1;

    value_t sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (size_t i = 0; i < static_cast<size_t>(n); i++) {
        value_t x = k[i].lowPrice;
        value_t y = k[i].highPrice;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    value_t null_price = Null<price_t>();
    value_t denominator = n * sum_x2 - sum_x * sum_x;
    dst[n - 1] = denominator == 0.0 ? null_price : (n * sum_xy - sum_x * sum_y) / denominator;

    for (size_t i = n; i < total; i++) {
        value_t old_x = k[i - n].lowPrice;
        value_t old_y = k[i - n].highPrice;
        value_t new_x = k[i].lowPrice;
        value_t new_y = k[i].highPrice;

        sum_x += new_x - old_x;
        sum_y += new_y - old_y;
        sum_xy += new_x * new_y - old_x * old_y;
        sum_x2 += new_x * new_x - old_x * old_x;

        denominator = n * sum_x2 - sum_x * sum_x;
        dst[i] = denominator == 0.0 ? null_price : (n * sum_xy - sum_x * sum_y) / denominator;
    }
}

Indicator HKU_API RSRS_BETA(int n) {
    auto p = make_shared<IRSRSBeta>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API RSRS_BETA(const KData& kdata, int n) {
    auto p = make_shared<IRSRSBeta>();
    p->setParam<int>("n", n);
    p->setContext(kdata);
    return Indicator(p);
}

}  // namespace hku