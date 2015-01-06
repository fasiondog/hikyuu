/*
 * Macd.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "Macd.h"
#include "../crt/EMA.h"

namespace hku {

Macd::Macd(int n1, int n2, int n3)
: IndicatorImp(0, 3) {
    addParam<int>("n1", n1);
    addParam<int>("n2", n2);
    addParam<int>("n3", n3);
}

Macd::Macd(const Indicator& data, int n1, int n2, int n3)
: IndicatorImp(data, 0, 3) {
    addParam<int>("n1", n1);
    addParam<int>("n2", n2);
    addParam<int>("n3", n3);

    size_t total = data.size();
    if (total <= discard()) {
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

    for (size_t  i = 1; i < total; ++i) {
        ema1 = data[i] * m1 + ema1 - ema1 * m1;
        ema2 = data[i] * m2 + ema2 - ema2 * m2;
        diff = ema1 - ema2;
        dea = diff * m3 + dea - dea * m3;
        bar = diff - dea;
        _set(bar, i, 0);
        _set(diff, i, 1);
        _set(dea, i, 2);
    }
}

Macd::~Macd() {

}

string Macd::name() const {
    return "MACD";
}

IndicatorImpPtr Macd::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new Macd(ind, getParam<int>("n1"),
            getParam<int>("n2"), getParam<int>("n3")));
}

Indicator HKU_API MACD(int n1, int n2, int n3) {
    return Indicator(IndicatorImpPtr(new Macd(n1, n2, n3)));
}

Indicator HKU_API MACD(const Indicator& data, int n1, int n2, int n3) {
    return Indicator(IndicatorImpPtr(new Macd(data, n1, n2, n3)));
}

} /* namespace hku */
