/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMrr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMrr)
#endif

namespace hku {

IMrr::IMrr() : IndicatorImp("MRR", 1) {}

IMrr::~IMrr() {}

bool IMrr::check() {
    return true;
}

void IMrr::_calculate(const Indicator& ind) {
    m_discard = 0;
    for (size_t i = 0, len = ind.discard(); i < len; i++) {
        _set(0.0, i);
    }

    size_t total = ind.size();
    if (ind.discard() < total) {
        _set(0., ind.discard());
    }

    price_t pre_min = ind[ind.discard()];
    for (size_t i = ind.discard() + 1; i < total; i++) {
        if (ind[i] <= pre_min || pre_min == 0.) {
            _set(0., i);
        } else {
            _set((ind[i] / pre_min - 1.0) * 100., i);
        }
        if (ind[i] < pre_min) {
            pre_min = ind[i];
        }
    }
}

Indicator HKU_API MRR() {
    IndicatorImpPtr p = make_shared<IMrr>();
    return Indicator(p);
}

}  // namespace hku