/*
 * ISign.cpp
 *
 * Copyright (c) 2019 fasiondog 
 * 
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#include "ISign.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISign)
#endif


namespace hku {

ISign::ISign() : IndicatorImp("SGN", 1) {

}

ISign::~ISign() {

}

bool ISign::check() {
    return true;
}

void ISign::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    for (size_t i = m_discard; i < total; i++) {
        if (ind[i] > 0) {
            _set(1, i);
        } else if (ind[i] < 0) {
            _set(-1, i);
        } else {
            _set(0, i);
        }
    }

    return;
}

Indicator HKU_API SGN() {
    return Indicator(make_shared<ISign>());
}

} /* namespace hku */