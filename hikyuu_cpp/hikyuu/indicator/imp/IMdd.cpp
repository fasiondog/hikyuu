/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMdd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMdd)
#endif

namespace hku {

IMdd::IMdd() : IndicatorImp("MDD", 1) {}

IMdd::~IMdd() {}

bool IMdd::check() {
    return true;
}

void IMdd::_calculate(const Indicator &ind) {
    m_discard = 0;
    for (size_t i = 0, len = ind.discard(); i < len; i++) {
        _set(0.0, i);
    }

    size_t total = ind.size();
    if (ind.discard() < total) {
        _set(0., ind.discard());
    }

    auto const *src = ind.data();
    auto *dst = this->data();
    price_t pre_max = ind[ind.discard()];
    for (size_t i = ind.discard() + 1; i < total; i++) {
        dst[i] = (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] / pre_max - 1.0) * 100.0;
        if (src[i] > pre_max) {
            pre_max = src[i];
        }
    }
}

Indicator HKU_API MDD() {
    IndicatorImpPtr p = make_shared<IMdd>();
    return Indicator(p);
}

}  // namespace hku