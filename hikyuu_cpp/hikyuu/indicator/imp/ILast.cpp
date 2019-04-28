/*
 * ILast.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#include "ILast.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILast)
#endif


namespace hku {

ILast::ILast() : IndicatorImp("LAST", 1) {
    setParam<int>("m", 10);
    setParam<int>("n", 5);
}

ILast::~ILast() {

}

bool ILast::check() {
    if (getParam<int>("m") < 0 || getParam<int>("n") < 0) {
        HKU_ERROR("Invalid param! (m>=0 and n>=0) "
                  << m_params << " [ILast::check]");
        return false;
    }
    return true;
}

void ILast::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        return;
    }

    int m = getParam<int>("m");
    int n = getParam<int>("n");
    if (0 == m) {
        m = total;
    }

    if (0 == n) {
        n = total;
    }

    if (m < n) {
        int tmp = m;
        m = n;
        n = tmp;
    }

    m_discard = ind.discard() + m;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t exist = 0;
    size_t pre_pos = m_discard;
    for (size_t i = ind.discard(); i <= m_discard; i++) {
         if (ind[i] != 0) {
             pre_pos = i;
             exist = 1;
        }
    }

    _set(exist, m_discard);

    for (size_t i = m_discard + 1; i < total-1; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            exist = ind[j] != 0 ? 1 : 0;
        }
        if (ind[i] != 0) {
            pre_pos = i;
            exist = 1;
        }
        _set(exist, i);
    }

    size_t start_pos = total - n;
    exist = 0;
    for (size_t i = start_pos; i < total; i++) {
        if (ind[i] != 0) {
            exist = 1;
            break;
        }
    }
    _set(exist, total-1);
}


Indicator HKU_API LAST(int m, int n) {
    IndicatorImpPtr p = make_shared<ILast>();
    p->setParam<int>("m", m);
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
