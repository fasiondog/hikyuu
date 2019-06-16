/*
 * ISum.cpp
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#include "ISum.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISum)
#endif


namespace hku {

ISum::ISum() : IndicatorImp("SUM", 1) {
    setParam<int>("n", 20);
}

ISum::~ISum() {

}

bool ISum::check() {
    int n = getParam<int>("n");
    if (n < 0) {
        HKU_ERROR("Invalid param[n] ! (n >= 0) {}", m_params);
        return false;
    }
    return true;
}

void ISum::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (ind.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = ind.discard();
        price_t sum = 0;
        for (size_t i = m_discard; i < total; i++) {
            sum += ind[i];
            _set(sum, i);
        }
        return;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t startPos = ind.discard();
    price_t sum = 0.0;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        sum += ind[i];
    }

    if (first_end >= 1) {
        _set(sum, first_end - 1);
    }
    
    for (size_t i = first_end; i < total; ++i) {
        sum = ind[i] + sum - ind[i-n];
        _set(sum, i);
    }

    return;
}

Indicator HKU_API SUM(int n) {
    IndicatorImpPtr p = make_shared<ISum>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
