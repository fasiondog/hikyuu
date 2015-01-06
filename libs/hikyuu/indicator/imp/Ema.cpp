/*
 * Ema.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "Ema.h"

namespace hku {

Ema::Ema(int n): IndicatorImp(0, 1) {
    addParam<int>("n", n);
    if (n <= 0) {
        HKU_ERROR("Invalid param[n] must > 0 ! [Ema::Ema]");
        return;
    }
}

Ema::Ema(const Indicator& indicator, int n): IndicatorImp(indicator, 0, 1) {
    addParam<int>("n", n);
    if (n <= 0) {
        HKU_ERROR("Invalid param[n] must > 0 ! [Ema::Ema]");
        return;
    }

    size_t total = indicator.size();
    if (total <= discard()) {
        return;
    }

    size_t startPos = discard();
    price_t ema = indicator[startPos];
    _set(ema, startPos);

    price_t multiplier = 2.0 / (n + 1);
    for (size_t i = startPos + 1; i < total; ++i) {
        ema = indicator[i] * multiplier + ema - ema * multiplier;
        _set(ema, i);
    }
}

Ema::~Ema() {

}

string Ema::name() const {
    return "EMA";
}

IndicatorImpPtr Ema::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new Ema(ind, getParam<int>("n")));
}

Indicator HKU_API EMA(int n) {
    return Indicator(IndicatorImpPtr(new Ema(n)));
}

Indicator HKU_API EMA(const Indicator& data, int n) {
    return Indicator(IndicatorImpPtr(new Ema(data, n)));
}

} /* namespace hku */
