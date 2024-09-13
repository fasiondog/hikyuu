/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "IDma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDma)
#endif

namespace hku {

IDma::IDma() : IndicatorImp("DMA") {}

IDma::IDma(const Indicator& ref_ind) : IndicatorImp("DMA"), m_ref_a(ref_ind) {}

IDma::~IDma() {}

void IDma::_checkParam(const string& name) const {}

IndicatorImpPtr IDma::_clone() {
    auto p = make_shared<IDma>();
    p->m_ref_a = m_ref_a.clone();
    return p;
}

void IDma::_calculate(const Indicator& ind) {
    auto k = getContext();
    m_ref_a.setContext(k);
    Indicator ref = m_ref_a;
    if (m_ref_a.size() != ind.size()) {
        ref = ALIGN(m_ref_a, ind);
    }

    size_t total = ind.size();
    _readyBuffer(total, 1);
    HKU_IF_RETURN(total == 0, void());

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

Indicator HKU_API DMA(const Indicator& x, const Indicator& a) {
    auto p = make_shared<IDma>(a);
    Indicator result(p);
    return result(x);
}

}  // namespace hku