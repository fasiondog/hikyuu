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

IRocp::IRocp() : IndicatorImp("ROCP", 1) {
    setParam<int>("n", 10);
}

IRocp::~IRocp() {}

void IRocp::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IRocp::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");

    m_discard = ind.discard() + n;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (0 == n) {
        price_t pre_price = src[m_discard];
        if (pre_price != 0.0) {
            dst[m_discard] = 0.0;
            for (size_t i = m_discard + 1; i < total; i++) {
                dst[i] = (src[i] - pre_price) / pre_price;
            }
        } else {
            for (size_t i = m_discard; i < total; i++) {
                dst[i] = 0.0;
            }
        }
        return;
    }

    for (size_t i = m_discard; i < total; i++) {
        price_t pre_price = src[i - n];
        dst[i] = (pre_price != 0.0) ? (src[i] - pre_price) / pre_price : 0.0;
    }
}

void IRocp::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step) {
        return;
    } else {
        start = curPos - step;
    }

    _set(ind[start] != 0.0 ? (ind[curPos] - ind[start]) / ind[start] : 0.0, curPos);
}

Indicator HKU_API ROCP(int n) {
    IndicatorImpPtr p = make_shared<IRocp>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ROCP(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IRocp>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
