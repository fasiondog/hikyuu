/*
 * IBarsSince.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "IBarsSince.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsSince)
#endif


namespace hku {

IBarsSince::IBarsSince() : IndicatorImp("BARSSINCE", 1) {

}

IBarsSince::~IBarsSince() {

}

bool IBarsSince::check() {
    return true;
}

void IBarsSince::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    bool found = false;
    size_t pos = m_discard;
    for (size_t i = m_discard; i < total; ++i) {
        if (found) {
            _set(i - pos, i);
        } else {
            if (ind[i] != 0.0) {
                found = true;
                pos = i;
                _set(0, i);
            }
        }
    }

    m_discard = pos;
}


Indicator HKU_API BARSSINCE() {
    return Indicator(make_shared<IBarsSince>());
}


} /* namespace hku */
