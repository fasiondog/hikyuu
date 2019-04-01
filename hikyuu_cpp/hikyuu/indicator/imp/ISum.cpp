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
        HKU_ERROR("Invalid param[n] ! (n >= 0) " << m_params
                << " [ISum::calculate]");
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

    int n = getParam<int>("n");
    if (0 == n) {
        m_discard = ind.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

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

    for (size_t i = m_discard; i < total; i++) {
        price_t sum = 0;
        for (size_t j = i + 1 - n; j <= i; j++) {
            sum += ind[j];
        }
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
