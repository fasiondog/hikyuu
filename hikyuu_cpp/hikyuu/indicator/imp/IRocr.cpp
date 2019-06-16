/*
 * IRocr.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "IRocr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRocr)
#endif


namespace hku {

IRocr::IRocr(): IndicatorImp("ROCR", 1) {
    setParam<int>("n", 10);
}

IRocr::~IRocr() {

}

bool IRocr::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param[n] ! (n >= 1) {}", m_params);
        return false;
    }

    return true;
}

void IRocr::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");

    m_discard = ind.discard() + n;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    for (size_t i = m_discard; i < total; i++) {
        price_t pre_price = ind[i-n];
        if (pre_price != 0.0) {
            _set(ind[i] / pre_price, i);
        } else {
            _set(0.0, i);
        }
    }
}


Indicator HKU_API ROCR(int n) {
    IndicatorImpPtr p = make_shared<IRocr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
