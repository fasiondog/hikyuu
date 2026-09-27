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
    m_is_serial = true;
    setParam<int>("n", 2);
}

IBackset::~IBackset() {}

void IBackset::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    }
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

    // Scan backwards, keeping how many more bars a hit still backfills, so that a window reaching
    // below the n-th bar is not overwritten afterwards.
    size_t fill = 0;
    for (size_t i = total; i-- > m_discard;) {
        if (src[i] != 0.0) {
            fill = n;
        }
        dst[i] = fill > 0 ? 1.0 : 0.0;
        if (fill > 0) {
            fill--;
        }
    }
}

void IBackset::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    // BACKSET is not causal: the bar itself is decided by it and the following step - 1 bars.
    size_t end = curPos + step;
    if (end > ind.size()) {
        end = ind.size();
    }

    bool found = false;
    for (size_t i = curPos; i < end; i++) {
        if (ind[i] != 0.0) {
            found = true;
            break;
        }
    }
    _set(found ? 1.0 : 0.0, curPos);
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
