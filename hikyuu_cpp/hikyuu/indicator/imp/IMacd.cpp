/*
 * IMacd.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "IMacd.h"
#include "../crt/EMA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMacd)
#endif

namespace hku {

IMacd::IMacd() : IndicatorImp("MACD", 3) {
    setParam<int>("n1", 12);
    setParam<int>("n2", 26);
    setParam<int>("n3", 9);
}

IMacd::~IMacd() {}

bool IMacd::check() {
    return getParam<int>("n1") > 0 && getParam<int>("n2") > 0 && getParam<int>("n3") > 0;
}

void IMacd::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 3);

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    int n3 = getParam<int>("n3");

    m_discard = data.discard();
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    price_t m1 = 2.0 / (n1 + 1);
    price_t m2 = 2.0 / (n2 + 1);
    price_t m3 = 2.0 / (n3 + 1);
    price_t ema1 = data[0];
    price_t ema2 = data[0];
    price_t diff = 0.0;
    price_t dea = 0.0;
    price_t bar = 0.0;
    _set(bar, 0, 0);
    _set(diff, 0, 1);
    _set(dea, 0, 2);

    for (size_t i = 1; i < total; ++i) {
        ema1 = (data[i] - ema1) * m1 + ema1;
        ema2 = (data[i] - ema2) * m2 + ema2;
        diff = ema1 - ema2;
        dea = diff * m3 + dea - dea * m3;
        bar = diff - dea;
        _set(bar, i, 0);
        _set(diff, i, 1);
        _set(dea, i, 2);
    }
}

Indicator HKU_API MACD(int n1, int n2, int n3) {
    IndicatorImpPtr p = make_shared<IMacd>();
    p->setParam<int>("n1", n1);
    p->setParam<int>("n2", n2);
    p->setParam<int>("n3", n3);
    return Indicator(p);
}

Indicator HKU_API MACD(const Indicator& data, int n1, int n2, int n3) {
    return MACD(n1, n2, n3)(data);
}

} /* namespace hku */
