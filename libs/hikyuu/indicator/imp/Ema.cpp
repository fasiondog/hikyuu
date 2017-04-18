/*
 * Ema.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "Ema.h"

namespace hku {

Ema::Ema(): IndicatorImp("EMA", 1) {
    setParam<int>("n", 22);
}

Ema::~Ema() {

}

bool Ema::check() {
    int n = getParam<int>("n");
    if (n <= 0) {
        HKU_ERROR("Invalid param[n] must > 0 ! [Ema::check]");
        return false;
    }

    return true;
}

void Ema::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();

    int n = getParam<int>("n");

    m_discard = indicator.discard();
    if (total <= m_discard) {
        return;
    }

    size_t startPos = discard();
    price_t ema = indicator[startPos];
    _set(ema, startPos);

    price_t multiplier = 2.0 / (n + 1);
    for (size_t i = startPos + 1; i < total; ++i) {
        ema = (indicator[i] - ema) * multiplier + ema;
        _set(ema, i);
    }
}


Indicator HKU_API EMA(int n) {
    IndicatorImpPtr p = make_shared<Ema>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API EMA(const Indicator& data, int n) {
    IndicatorImpPtr p = make_shared<Ema>();
    p->setParam<int>("n", n);
    p->calculate(data);
    return Indicator(p);
}

} /* namespace hku */
