/*
 * IBackset.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#include "IBackset.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBackset)
#endif


namespace hku {

IBackset::IBackset() : IndicatorImp("BACKSET", 1) {
    setParam<int>("n", 2);
}

IBackset::~IBackset() {

}

bool IBackset::check() {
    if (getParam<int>("n") < 1) {
        HKU_ERROR("Invalid param! (n>=1) {}", m_params);
        return false;
    }
    return true;
}

void IBackset::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t i = total;
    size_t end_i = m_discard + n;
    if (end_i > total) {
        end_i = total;
    }
    while (i-- > end_i) {
        if (ind[i] != 0.0) {
            _set(1.0, i);
            size_t j = i;
            size_t end_j = i - n + 1;
            while (j-- > end_j) {
                if (get(j) != 1.0) {
                    _set(1.0, j);
                }
            }
        } else {
            if (get(i) != 1.0) {
                _set(0.0, i);
            }
        }
    }

    //i = end_i - 1;
    while (true) {
        if (ind[i] != 0.0) {
            for (size_t j = m_discard; j <= i; j++) {
                _set(1.0, j);
            }
            break;
        } else {
            _set(0.0, i);
            if (i == m_discard) {
                break;
            }
            i--;
        }
    }
}


Indicator HKU_API BACKSET(int n) {
    IndicatorImpPtr p = make_shared<IBackset>();
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
