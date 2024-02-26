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
    return getParam<int>("n") >= 0;
}

void IEvery::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (0 == n) {
        m_discard = ind.discard();
        for (size_t i = m_discard; i < total; i++) {
            price_t every = 1.0;
            for (size_t j = m_discard; j <= i; j++) {
                if (src[j] == 0.0) {
                    every = 0.0;
                    break;
                }
            }
            dst[i] = every;
        }
        return;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t every = 1;
    size_t pre_pos = m_discard;
    for (size_t i = ind.discard(); i <= m_discard; i++) {
        if (src[i] == 0) {
            pre_pos = i;
            every = 0;
        }
    }

    dst[m_discard] = every;
    for (size_t i = m_discard + 1; i < total - 1; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            every = src[j] == 0 ? 0 : 1;
        }
        if (src[i] == 0) {
            pre_pos = i;
            every = 0;
        }
        dst[i] = every;
    }

    size_t start_pos = total - n;
    every = 1;
    for (size_t i = start_pos; i < total; i++) {
        if (src[i] == 0) {
            every = 0;
            break;
        }
    }
    dst[total - 1] = every;
}

void IEvery::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step - 1) {
        return;
    } else {
        start = curPos + 1 - step;
    }

    price_t every = 1.0;
    for (size_t i = start; i <= curPos; i++) {
        if (ind[i] == 0.0) {
            every = 0.0;
            break;
        }
    }
    _set(every, curPos);
}

Indicator HKU_API EVERY(int n) {
    IndicatorImpPtr p = make_shared<IEvery>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API EVERY(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IEvery>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
