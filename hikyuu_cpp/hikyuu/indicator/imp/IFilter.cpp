/*
 * IFilter.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "IFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFilter)
#endif


namespace hku {

IFilter::IFilter() : IndicatorImp("FILTER", 1) {
    setParam<int>("n", 5);
}

IFilter::~IFilter() {

}

bool IFilter::check() {
    if (getParam<int>("n") < 1) {
        HKU_ERROR("Invalid param! (n>=1) "
                  << m_params << " [IFilter::check]");
        return false;
    }
    return true;
}

void IFilter::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    /*std::cout << "n: " << n << std::endl;
    std::cout << "total: " << total << std::endl;
    
    bool have = false;
    size_t pre_pos = m_discard;
    size_t start_pos = m_discard + n < total ? m_discard + n : total;
    for (size_t i = m_discard; i < start_pos; i++) {
        if (ind[i] != 0.0) {
            have = true;
            pre_pos = i;
            _set(1.0, i);
        } else {
            _set(0.0, i);
        }
    }

    std::cout << "start_pos: " << start_pos << std::endl;

    for (size_t i = start_pos; i < total; i++) {
        size_t j = i - n;
        std::cout << i << "," << j << ", " << have << ", " << pre_pos << std::endl;
        if (pre_pos < j) {
            have = ind[j] == 0.0 ? false : true;
            pre_pos = j;
        }
        
        if (have) {
            _set(0.0, i);
        } else {
            _set(ind[i] != 0.0 ? 1.0 : 0.0, i);
        }
        
        if (ind[i] != 0.0) {
            have = true;
            pre_pos = i;
        }
    }*/


    size_t i = m_discard;
    size_t len = total - m_discard;
    len = len > n + 1 ? len - n - 1 : len;
    while (i < total) {
        if (ind[i] == 0.0) {
            _set(0.0, i);
            i++;
        } else {
            _set(1.0, i);
            size_t end = i + n + 1;
            if (end > total) {
                end = total;
            }
            for (size_t j = i + 1; j < end; j++) {
                _set(0, j);
            }
            i = end;
        }
    }
}


Indicator HKU_API FILTER(int n) {
    IndicatorImpPtr p = make_shared<IFilter>();
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
