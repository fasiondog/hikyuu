/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#include "IBeta.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBeta)
#endif

namespace hku {

IBeta::IBeta() : Indicator2InImp("BETA", 1) {
    setParam<int>("n", 10);
}

IBeta::IBeta(const Indicator& ref_ind, int n, bool fill_null)
: Indicator2InImp("BETA", ref_ind, fill_null, 1) {
    setParam<int>("n", n);
}

IBeta::~IBeta() {}

void IBeta::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

void IBeta::_calculate(const Indicator& ind) {
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

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[startPos];
    value_t ky = datay[startPos];
    value_t ex = 0.0, ey = 0.0, exy = 0.0, vary = 0.0, cov = 0.0;
    value_t ey2 = 0.0;
    value_t ix, iy;

    auto* dst = this->data();
    size_t first_end = startPos + n >= total ? total : startPos + n;

    for (size_t i = startPos + 1; i < first_end; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += ix;
        ey += iy;
        ey2 += iy * iy;
        exy += ix * iy;
    }

    value_t null_price = Null<price_t>();
    vary = ey2 - ey * ey / n;
    cov = exy - ex * ey / n;
    dst[first_end - 1] = vary == 0.0 ? null_price : cov / vary;

    for (size_t i = first_end; i < total; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += datax[i] - datax[i - n];
        ey += datay[i] - datay[i - n];
        value_t preiy = datay[i - n] - ky;
        ey2 += iy * iy - preiy * preiy;
        exy += ix * iy - (datax[i - n] - kx) * preiy;
        vary = (ey2 - ey * ey / n);
        cov = (exy - ex * ey / n);
        dst[i] = vary == 0.0 ? null_price : cov / vary;
    }
}

bool IBeta::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

size_t IBeta::min_increment_start() const {
    return getParam<int>("n");
}

void IBeta::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    size_t startPos = start_pos - n;

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[startPos];
    value_t ky = datay[startPos];
    value_t ex = 0.0, ey = 0.0, exy = 0.0, vary = 0.0, cov = 0.0;
    value_t ey2 = 0.0;
    value_t ix, iy;

    auto* dst = this->data();
    size_t first_end = start_pos;

    for (size_t i = startPos + 1; i < first_end; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += ix;
        ey += iy;
        ey2 += iy * iy;
        exy += ix * iy;
    }

    vary = ey2 - ey * ey / n;
    cov = exy - ex * ey / n;

    value_t null_price = Null<price_t>();
    for (size_t i = first_end; i < total; i++) {
        ix = datax[i] - kx;
        iy = datay[i] - ky;
        ex += datax[i] - datax[i - n];
        ey += datay[i] - datay[i - n];
        value_t preiy = datay[i - n] - ky;
        ey2 += iy * iy - preiy * preiy;
        exy += ix * iy - (datax[i - n] - kx) * preiy;
        vary = (ey2 - ey * ey / n);
        cov = (exy - ex * ey / n);
        dst[i] = vary == 0.0 ? null_price : cov / vary;
    }
}

Indicator HKU_API BETA(const Indicator& ref_ind, int n, bool fill_null) {
    return Indicator(make_shared<IBeta>(ref_ind, n, fill_null));
}

Indicator HKU_API BETA(const Indicator& ind1, const Indicator& ind2, int n, bool fill_null) {
    auto p = make_shared<IBeta>(ind2, n, fill_null);
    Indicator result(p);
    return result(ind1);
}

}  // namespace hku