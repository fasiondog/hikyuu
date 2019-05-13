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
        HKU_ERROR("Invalid param! (n>=1) "
                  << m_params << " [IBackset::check]");
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
    size_t end_i = m_discard + n + 1;
    std::cout << "end_i: " << end_i << std::endl;
    while (i > end_i) {
        i--;
        std::cout << "i: " << i << std::endl;
        if (ind[i] != 0.0) {
            _set(1.0, i);
            size_t j = i;
            size_t end_j = i - n - 1;
            while (j > end_j) {
                j--;
                std::cout << "i: " << i << ", j: " << j << "end_j: " << end_j << std::endl;
                _set(1.0, j);
            }
            i = end_j + 1;
        } else {
            _set(0.0, i);
        }
    }

    i = end_i - 1;
    while (true) {
        std::cout << "**** " << i <<std::endl;
        if (ind[i] != 0.0) {
            for (size_t j = m_discard; j < i; j++) {
                std::cout << "i: " << i << ", j: " << j << std::endl;
                _set(1.0, j);
            }
            break;
        } else {
            std::cout << i << std::endl;
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
