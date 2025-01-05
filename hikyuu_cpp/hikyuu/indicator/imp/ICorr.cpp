/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "ICorr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICorr)
#endif

namespace hku {

ICorr::ICorr() : IndicatorImp("CORR") {
    setParam<int>("n", 10);
    setParam<bool>("fill_null", true);
}

ICorr::ICorr(const Indicator& ref_ind, int n, bool fill_null)
: IndicatorImp("CORR"), m_ref_ind(ref_ind) {
    setParam<int>("n", n);
    setParam<bool>("fill_null", fill_null);
}

ICorr::~ICorr() {}

void ICorr::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

IndicatorImpPtr ICorr::_clone() {
    auto p = make_shared<ICorr>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void ICorr::_calculate(const Indicator& ind) {
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

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
    }

    m_discard = std::max(ind.discard(), ref.discard());
    size_t startPos = m_discard;
    size_t first_end = startPos + n >= total ? total : startPos + n;

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[m_discard];
    value_t ky = datay[m_discard];
    value_t ex = 0.0, ey = 0.0, exy = 0.0, varx = 0.0, vary = 0.0, cov = 0.0;
    value_t ex2 = 0.0, ey2 = 0.0;
    value_t ix, iy;

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    for (size_t i = startPos + 1; i < first_end; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += ix;
        ey += iy;
        value_t powx2 = ix * ix;
        value_t powy2 = iy * iy;
        value_t powxy = ix * iy;
        exy += powxy;
        ex2 += powx2;
        ey2 += powy2;
        size_t nobs = i - startPos;
        varx = ex2 - powx2 / nobs;
        vary = ey2 - powy2 / nobs;
        cov = exy - powxy / nobs;
        dst0[i] = cov / std::sqrt(varx * vary);
        dst1[i] = cov / (nobs - 1);
    }

    for (size_t i = first_end; i < total; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += datax[i] - datax[i - n];
        ey += datay[i] - datay[i - n];
        value_t preix = datax[i - n] - kx;
        value_t preiy = datay[i - n] - ky;
        ex2 += ix * ix - preix * preix;
        ey2 += iy * iy - preiy * preiy;
        exy += ix * iy - preix * preiy;
        varx = (ex2 - ex * ex / n);
        vary = (ey2 - ey * ey / n);
        cov = (exy - ex * ey / n);
        dst0[i] = cov / std::sqrt(varx * vary);
        dst1[i] = cov / (n - 1);
    }

    // 修正 discard
    m_discard = (m_discard + 2 < total) ? m_discard + 2 : total;
}

Indicator HKU_API CORR(const Indicator& ref_ind, int n, bool fill_null) {
    return Indicator(make_shared<ICorr>(ref_ind, n, fill_null));
}

Indicator HKU_API CORR(const Indicator& ind1, const Indicator& ind2, int n, bool fill_null) {
    auto p = make_shared<ICorr>(ind2, n, fill_null);
    Indicator result(p);
    return result(ind1);
}

}  // namespace hku