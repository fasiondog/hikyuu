/*
 * IRocp.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "IRocp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRocp)
#endif


namespace hku {

IRocp::IRocp(): IndicatorImp("ROCP", 1) {
    setParam<int>("n", 10);
}

IRocp::~IRocp() {

}

bool IRocp::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param[n] ! (n >= 1) " << m_params
                << " [IRocp::calculate]");
        return false;
    }

    return true;
}

void IRocp::_calculate(const Indicator& ind) {
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
            _set((ind[i] - pre_price) / pre_price, i);
        } else {
            _set(0.0, i);
        }
    }
}


Indicator HKU_API ROCP(int n) {
    IndicatorImpPtr p = make_shared<IRocp>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
