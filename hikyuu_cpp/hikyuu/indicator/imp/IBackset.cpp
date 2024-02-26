/*
 * IBackset.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#include "IBackset.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBackset)
#endif

namespace hku {

IBackset::IBackset() : IndicatorImp("BACKSET", 1) {
    setParam<int>("n", 2);
}

IBackset::~IBackset() {}

bool IBackset::check() {
    return getParam<int>("n") >= 1;
}

void IBackset::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t i = total;
    size_t end_i = m_discard + n;
    if (end_i > total) {
        end_i = total;
    }
    while (i-- > end_i) {
        if (src[i] != 0.0) {
            dst[i] = 1.0;
            size_t j = i;
            size_t end_j = i - n + 1;
            while (j-- > end_j) {
                dst[j] = 1.0;
            }
        } else {
            if (dst[i] != 1.0) {
                dst[i] = 0.0;
            }
        }
    }

    // i = end_i - 1;
    while (true) {
        if (src[i] != 0.0) {
            for (size_t j = m_discard; j <= i; j++) {
                dst[j] = 1.0;
            }
            break;
        } else {
            dst[i] = 0.0;
            if (i == m_discard) {
                break;
            }
            i--;
        }
    }
}

void IBackset::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (ind[curPos] == 0.0) {
        for (size_t i = start; i <= curPos; i++) {
            _set(0.0, curPos);
        }
    } else {
        for (size_t i = start; i <= curPos; i++) {
            _set(1.0, curPos);
        }
    }
}

Indicator HKU_API BACKSET(int n) {
    IndicatorImpPtr p = make_shared<IBackset>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API BACKSET(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IBackset>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
