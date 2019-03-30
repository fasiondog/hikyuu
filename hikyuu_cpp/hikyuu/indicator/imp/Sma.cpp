/*
 * Sma.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#include "Sma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::Sma)
#endif


namespace hku {

Sma::Sma(): IndicatorImp("SMA", 1) {
    setParam<int>("n", 22);
}

Sma::~Sma() {

}

bool Sma::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param! (n >= 1) [Sma::check]");
        return false;
    }

    return true;
}

void Sma::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    size_t startPos = m_discard;
    price_t sum = 0.0;
    size_t count = 1;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        sum += indicator[i];
        _set(sum/count++, i);
    }

    for (size_t i = first_end; i < total; ++i) {
        sum = indicator[i] + sum - indicator[i-n];
        _set(sum/n, i);
    }
}


Indicator HKU_API SMA(int n) {
    IndicatorImpPtr p = make_shared<Sma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SMA(const Indicator& ind, int n){
    return SMA(n)(ind);
}

} /* namespace hku */
