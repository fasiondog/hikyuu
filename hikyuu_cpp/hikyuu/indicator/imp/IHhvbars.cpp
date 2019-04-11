/*
 * IHhvbars.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#include "IHhvbars.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IHhvbars)
#endif


namespace hku {

IHhvbars::IHhvbars() : IndicatorImp("HHVBARS", 1) {
    setParam<int>("n", 20);
}

IHhvbars::~IHhvbars() {

}

bool IHhvbars::check() {
    if (getParam<int>("n") < 0) {
        HKU_ERROR("Invalid param! (n>=>0) "
                  << m_params << " [IHhvbars::check]");
        return false;
    }
    return true;
}

void IHhvbars::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    int n = getParam<int>("n");
    m_discard = ind.discard();
    if (0 == n) {
        n = total - m_discard;
    }

    price_t max = ind[m_discard];
    size_t pre_pos = m_discard;
    size_t start_pos = m_discard + n;
    for (size_t i = m_discard; i < start_pos; i++) {
         if (ind[i] > max) {
            max = ind[i];
            pre_pos = i;
        }
        _set(i - pre_pos, i);
    }

    for (size_t i = start_pos; i < total; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            max = ind[j];
        }
        if (ind[i] > max) {
            max = ind[i];
            pre_pos = i;
        }
        _set(i - pre_pos, i);
    }
}


Indicator HKU_API HHVBARS(int n) {
    IndicatorImpPtr p = make_shared<IHhvbars>();
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
