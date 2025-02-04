/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "IDma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDma)
#endif

namespace hku {

IDma::IDma() : Indicator2InImp("DMA") {}

IDma::IDma(const Indicator& ref_ind, bool fill_null)
: Indicator2InImp("DMA", ref_ind, fill_null, 1) {}

IDma::~IDma() {}

void IDma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    m_discard = std::max(ind.discard(), ref.discard());
    auto* y = this->data();
    const auto* a = ref.data();
    const auto* x = ind.data();
    y[m_discard] = x[m_discard];
    for (size_t i = m_discard + 1; i < total; i++) {
        if (std::isnan(y[i - 1])) {
            y[i] = x[i];
        } else {
            y[i] = a[i] * x[i] + (1 - a[i]) * y[i - 1];
        }
    }
}

Indicator HKU_API DMA(const Indicator& x, const Indicator& a, bool fill_null) {
    auto p = make_shared<IDma>(a, fill_null);
    Indicator result(p);
    return result(x);
}

}  // namespace hku