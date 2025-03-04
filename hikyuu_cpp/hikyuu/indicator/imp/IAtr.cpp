/*
 * IAtr.cpp
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#include "../crt/ATR.h"
#include "../crt/SLICE.h"
#include "IAtr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAtr)
#endif

namespace hku {

IAtr::IAtr() : IndicatorImp("ATR", 1) {
    setParam<int>("n", 14);
}

IAtr::IAtr(const KData& k, int n) : IndicatorImp("ATR", 1) {
    setParam<int>("n", n);
    setParam<KData>("kdata", k);
    IAtr::_calculate(Indicator());
}

IAtr::~IAtr() {}

void IAtr::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    }
}

void IAtr::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    int n = getParam<int>("n");
    m_discard = n + 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto* k = kdata.data();
    vector<value_t> buf(total);
    for (size_t i = 1; i < total; ++i) {
        value_t v1 = k[i].highPrice - k[i].lowPrice;
        value_t v2 = std::abs(k[i].highPrice - k[i - 1].closePrice);
        value_t v3 = std::abs(k[i].lowPrice - k[i - 1].closePrice);
        buf[i] = std::max(std::max(v1, v2), v3);
    }

    value_t sum = 0.0;
    for (size_t i = 1, end = n + 1; i < end; ++i) {
        sum += buf[i];
    }

    auto* dst = this->data();
    dst[n] = sum / n;

    for (size_t i = n + 1; i < total; ++i) {
        sum = buf[i] + sum - buf[i - n];
        dst[i] = sum / n;
    }
}

Indicator HKU_API ATR(int n) {
    auto p = make_shared<IAtr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ATR(const KData& kdata, int n) {
    return Indicator(make_shared<IAtr>(kdata, n));
}

} /* namespace hku */
