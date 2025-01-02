/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "IDma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDma)
#endif

namespace hku {

IDma::IDma() : IndicatorImp("DMA") {
    setParam<bool>("fill_null", true);
}

IDma::IDma(const Indicator& ref_ind, bool fill_null) : IndicatorImp("DMA"), m_ref_ind(ref_ind) {
    setParam<bool>("fill_null", fill_null);
}

IDma::~IDma() {}

void IDma::_checkParam(const string& name) const {}

IndicatorImpPtr IDma::_clone() {
    auto p = make_shared<IDma>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void IDma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 2);

    auto k = getContext();
    m_ref_ind.setContext(k);
    Indicator ref = m_ref_ind;
    auto dates = ref.getDatetimeList();
    if (dates.empty()) {
        if (ref.size() > ind.size()) {
            ref = SLICE(ref, ref.size() - ind.size(), ref.size());
        } else if (ref.size() < ind.size()) {
            ref = CVAL(ind, 0.) + ref;
        }
    } else if (m_ref_ind.size() != ind.size()) {
        ref = ALIGN(m_ref_ind, ind, getParam<bool>("fill_null"));
    }

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