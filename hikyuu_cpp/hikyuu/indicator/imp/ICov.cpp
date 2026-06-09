/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "ICov.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICov)
#endif

namespace hku {

ICov::ICov() : Indicator2InImp("COV") {
    setParam<int>("n", 10);
}

ICov::ICov(const Indicator& ref_ind, int n, bool fill_null)
: Indicator2InImp("COV", ref_ind, fill_null, 1) {
    setParam<int>("n", n);
}

ICov::~ICov() {}

void ICov::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

void ICov::_calculate(const Indicator& ind) {
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
    value_t ex = 0.0, ey = 0.0, exy = 0.0, cov = 0.0;
    value_t ix, iy;

    auto* dst0 = this->data(0);
    for (size_t i = startPos + 1; i < first_end; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += ix;
        ey += iy;
        exy += ix * iy;
    }

    cov = exy - ex * ey / n;
    dst0[first_end - 1] = cov / (n - 1);

    for (size_t i = first_end; i < total; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += datax[i] - datax[i - n];
        ey += datay[i] - datay[i - n];
        value_t preix = datax[i - n] - kx;
        value_t preiy = datay[i - n] - ky;
        exy += ix * iy - preix * preiy;
        cov = (exy - ex * ey / n);
        dst0[i] = cov / (n - 1);
    }
}

bool ICov::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

size_t ICov::min_increment_start() const {
    return getParam<int>("n");
}

void ICov::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    size_t startPos = start_pos - n;

    size_t first_end = start_pos;

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[startPos];
    value_t ky = datay[startPos];
    value_t ex = 0.0, ey = 0.0, exy = 0.0, cov = 0.0;
    value_t ix, iy;

    auto* dst0 = this->data(0);
    for (size_t i = startPos + 1; i < first_end; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += ix;
        ey += iy;
        exy += ix * iy;
    }

    cov = exy - ex * ey / n;

    for (size_t i = first_end; i < total; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += datax[i] - datax[i - n];
        ey += datay[i] - datay[i - n];
        value_t preix = datax[i - n] - kx;
        value_t preiy = datay[i - n] - ky;
        exy += ix * iy - preix * preiy;
        cov = (exy - ex * ey / n);
        dst0[i] = cov / (n - 1);
    }
}

Indicator HKU_API COV(const Indicator& ref_ind, int n, bool fill_null) {
    return Indicator(make_shared<ICov>(ref_ind, n, fill_null));
}

Indicator HKU_API COV(const Indicator& ind1, const Indicator& ind2, int n, bool fill_null) {
    auto p = make_shared<ICov>(ind2, n, fill_null);
    Indicator result(p);
    return result(ind1);
}

}  // namespace hku