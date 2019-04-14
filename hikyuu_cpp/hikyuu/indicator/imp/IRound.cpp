/*
 * IRound.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */

#include "../../utilities/util.h"
#include "IRound.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRound)
#endif


namespace hku {

IRound::IRound() : IndicatorImp("ROUND", 1) {
    setParam<int>("ndigits", 2);
}

IRound::~IRound() {

}

bool IRound::check() {
    if (getParam<int>("ndigits") < 0) {
        HKU_ERROR("Invalid param[ndigits] ! (n >= 0) " << m_params
                << " [IRound::check]");
        return false;
    }
    return true;
}

void IRound::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("ndigits");
    for (size_t i = m_discard; i < total; ++i) {
        _set(roundEx(data[i], n), i);
    }
}


Indicator HKU_API ROUND(int ndigits) {
    IndicatorImpPtr p = make_shared<IRound>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hku */
