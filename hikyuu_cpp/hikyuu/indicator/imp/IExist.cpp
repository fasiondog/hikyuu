/*
 * IExist.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

#include "IExist.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IExist)
#endif


namespace hku {

IExist::IExist() : IndicatorImp("EXIST", 1) {
    setParam<int>("n", 20);
}

IExist::~IExist() {

}

bool IExist::check() {
    if (getParam<int>("n") < 1) {
        HKU_ERROR("Invalid param! (n>=1) "
                  << m_params << " [IHhvbars::check]");
        return false;
    }
    return true;
}

void IExist::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t exist = 0;
    size_t pre_pos = m_discard;
    size_t start_pos = m_discard < total ? m_discard : total;
    for (size_t i = ind.discard(); i < start_pos; i++) {
         if (ind[i] != 0) {
             pre_pos = i;
             exist = 1;
        }
    }

    if (start_pos >= 1) {
        _set(exist, start_pos - 1);
    }

    for (size_t i = start_pos; i < total-1; i++) {
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

    start_pos = total - n;
    exist = 0;
    for (size_t i = start_pos; i < total; i++) {
        if (ind[i] != 0) {
            exist = 1;
            break;
        }
    }
    _set(exist, total-1);
}


Indicator HKU_API EXIST(int n) {
    IndicatorImpPtr p = make_shared<IExist>();
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
