/*
 * Atr.cpp
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#include "Atr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::Atr)
#endif


namespace hku {

Atr::Atr() : IndicatorImp("ATR", 1){
    setParam<int>("n", 14);
}

Atr::~Atr() {

}

bool Atr::check() {
    if (getParam<int>("n") < 1) {
        HKU_ERROR("Invalid param! (n>=1) ", m_params);
        return false;
    }
    return true;
}

void Atr::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
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

Indicator HKU_API ATR(int n) {
    IndicatorImpPtr p = make_shared<Atr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ATR(const Indicator& data, int n) {
    return ATR(n)(data);
}

} /* namespace hku */
