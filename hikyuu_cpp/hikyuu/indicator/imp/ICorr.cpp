/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "ICorr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICorr)
#endif

namespace hku {

ICorr::ICorr() : Indicator2InImp("CORR") {
    setParam<int>("n", 10);
}

ICorr::ICorr(const Indicator& ref_ind, int n, bool fill_null)
: Indicator2InImp("CORR", ref_ind, fill_null, 2) {
    setParam<int>("n", n);
}

ICorr::~ICorr() {}

void ICorr::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

void ICorr::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
    }

    size_t startPos = std::max(ind.discard(), ref.discard());
    m_discard = startPos + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t first_end = startPos + n >= total ? total : startPos + n;

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[startPos];
    value_t ky = datay[startPos];
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
        ex2 += ix * ix;
        ey2 += iy * iy;
        exy += ix * iy;
    }

    varx = ex2 - ex * ex / n;
    vary = ey2 - ey * ey / n;
    cov = exy - ex * ey / n;
    dst0[first_end - 1] = cov / std::sqrt(varx * vary);
    dst1[first_end - 1] = cov / (n - 1);

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