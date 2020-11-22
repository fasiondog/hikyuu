/*
 * Ema.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "Ema.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::Ema)
#endif

namespace hku {

Ema::Ema() : IndicatorImp("EMA", 1) {
    setParam<int>("n", 22);
}

Ema::~Ema() {}

bool Ema::check() {
    return getParam<int>("n") > 0;
}

void Ema::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
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
    return EMA(n)(data);
}

} /* namespace hku */
