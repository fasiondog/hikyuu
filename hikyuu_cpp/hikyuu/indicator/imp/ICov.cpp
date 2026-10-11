/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <climits>
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
        HKU_ASSERT(n == 0 || (n >= 2 && n < INT_MAX));
    }
}

void ICov::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    size_t startPos = std::max(ind.discard(), ref.discard());

    // n == 0: expanding-window sample covariance over [startPos, i]; pairs with any non-finite
    // value are excluded from both the sums and the running denominator.
    if (0 == n) {
        m_discard = startPos + 1;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        auto const* datax = ind.data();
        auto const* datay = ref.data();
        value_t kx = datax[startPos];
        value_t ky = datay[startPos];
        if (!std::isfinite(kx))
            kx = 0.0;
        if (!std::isfinite(ky))
            ky = 0.0;
        value_t ex = 0.0, ey = 0.0, exy = 0.0;
        size_t valid = 0;
        if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
            valid = 1;
        auto* dst0 = this->data(0);
        for (size_t i = startPos + 1; i < total; ++i) {
            if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
                value_t ix = datax[i] - kx;
                value_t iy = datay[i] - ky;
                ex += ix;
                ey += iy;
                exy += ix * iy;
                valid++;
            }
            if (valid < 2)
                continue;
            value_t nv = (value_t)valid;
            dst0[i] = (exy - ex * ey / nv) / (nv - 1);
        }
        return;
    }

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
    if (!std::isfinite(kx))
        kx = 0.0;
    if (!std::isfinite(ky))
        ky = 0.0;
    value_t ex = 0.0, ey = 0.0, exy = 0.0;
    size_t valid = 0;
    if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
        valid = 1;

    auto* dst0 = this->data(0);
    for (size_t i = startPos + 1; i < first_end; i++) {
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            exy += ix * iy;
            valid++;
        }
    }

    if (valid >= 2) {
        value_t nv = (value_t)valid;
        dst0[first_end - 1] = (exy - ex * ey / nv) / (nv - 1);
    }

    for (size_t i = first_end; i < total; i++) {
        bool out_ok = std::isfinite(datax[i - n]) && std::isfinite(datay[i - n]);
        if (out_ok) {
            value_t preix = datax[i - n] - kx;
            value_t preiy = datay[i - n] - ky;
            ex -= preix;
            ey -= preiy;
            exy -= preix * preiy;
            valid--;
        }
        bool in_ok = std::isfinite(datax[i]) && std::isfinite(datay[i]);
        if (in_ok) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            exy += ix * iy;
            valid++;
        }
        if (valid < 2)
            continue;
        value_t nv = (value_t)valid;
        dst0[i] = (exy - ex * ey / nv) / (nv - 1);
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
    if (!std::isfinite(kx))
        kx = 0.0;
    if (!std::isfinite(ky))
        ky = 0.0;
    value_t ex = 0.0, ey = 0.0, exy = 0.0;
    size_t valid = 0;
    if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
        valid = 1;

    auto* dst0 = this->data(0);
    for (size_t i = startPos + 1; i < first_end; i++) {
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            exy += ix * iy;
            valid++;
        }
    }

    for (size_t i = first_end; i < total; i++) {
        bool out_ok = std::isfinite(datax[i - n]) && std::isfinite(datay[i - n]);
        if (out_ok) {
            value_t preix = datax[i - n] - kx;
            value_t preiy = datay[i - n] - ky;
            ex -= preix;
            ey -= preiy;
            exy -= preix * preiy;
            valid--;
        }
        bool in_ok = std::isfinite(datax[i]) && std::isfinite(datay[i]);
        if (in_ok) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            exy += ix * iy;
            valid++;
        }
        if (valid < 2)
            continue;
        value_t nv = (value_t)valid;
        dst0[i] = (exy - ex * ey / nv) / (nv - 1);
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
