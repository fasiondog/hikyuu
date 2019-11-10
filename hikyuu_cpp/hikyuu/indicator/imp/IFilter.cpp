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

IFilter::~IFilter() {}

bool IFilter::check() {
    if (getParam<int>("n") < 1) {
        HKU_ERROR("Invalid param! (n>=1) {}", m_params);
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
    size_t i = m_discard;
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
