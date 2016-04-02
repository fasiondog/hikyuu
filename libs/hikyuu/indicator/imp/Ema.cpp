/*
 * Ema.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "Ema.h"

namespace hku {

Ema::Ema(): IndicatorImp("EMA") {
    setParam<int>("n", 22);
}

Ema::~Ema() {

}

void Ema::calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    _readyBuffer(total, 1);

    int n = getParam<int>("n");
    if (n <= 0) {
        HKU_ERROR("Invalid param[n] must > 0 ! [Ema::Ema]");
        return;
    }

    m_discard = indicator.discard();
    if (total <= m_discard) {
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


Indicator HKU_API EMA(int n) {
    IndicatorImpPtr p(new Ema());
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API EMA(const Indicator& data, int n) {
    IndicatorImpPtr p(new Ema());
    p->setParam<int>("n", n);
    p->calculate(data);
    return Indicator(p);
}

} /* namespace hku */
