/*
 * IEvery.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#include "IEvery.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IEvery)
#endif

namespace hku {

IEvery::IEvery() : IndicatorImp("EVERY", 1) {
    setParam<int>("n", 20);
}

IEvery::~IEvery() {}

bool IEvery::check() {
    if (getParam<int>("n") < 0) {
        HKU_ERROR("Invalid param! (n>=0) {}", m_params);
        return false;
    }
    return true;
}

void IEvery::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        return;
    }

    int n = getParam<int>("n");
    if (0 == n) {
        n = total;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t every = 1;
    size_t pre_pos = m_discard;
    for (size_t i = ind.discard(); i <= m_discard; i++) {
        if (ind[i] == 0) {
            pre_pos = i;
            every = 0;
        }
    }

    _set(every, m_discard);

    for (size_t i = m_discard + 1; i < total - 1; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            every = ind[j] == 0 ? 0 : 1;
        }
        if (ind[i] == 0) {
            pre_pos = i;
            every = 0;
        }
        _set(every, i);
    }

    size_t start_pos = total - n;
    every = 1;
    for (size_t i = start_pos; i < total; i++) {
        if (ind[i] == 0) {
            every = 0;
            break;
        }
    }
    _set(every, total - 1);
}

Indicator HKU_API EVERY(int n) {
    IndicatorImpPtr p = make_shared<IEvery>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
