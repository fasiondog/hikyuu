/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#include <climits>
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
        HKU_ASSERT(n == 0 || (n >= 2 && n < INT_MAX));
    }
}

void IBeta::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    size_t startPos = std::max(ind.discard(), ref.discard());
    value_t null_price = Null<price_t>();

    // n == 0: expanding-window beta = sample cov / pop var(y) over [startPos, i]; pairs with any
    // non-finite value are excluded from the sums and the running denominator.
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
        value_t ex = 0.0, ey = 0.0, ey2 = 0.0, exy = 0.0;
        size_t valid = 0;
        if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
            valid = 1;
        auto* dst = this->data();
        for (size_t i = startPos + 1; i < total; ++i) {
            if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
                value_t ix = datax[i] - kx;
                value_t iy = datay[i] - ky;
                ex += ix;
                ey += iy;
                ey2 += iy * iy;
                exy += ix * iy;
                valid++;
            }
            if (valid < 2)
                continue;
            value_t nv = (value_t)valid;
            value_t vary = ey2 - ey * ey / nv;
            value_t cov = exy - ex * ey / nv;
            dst[i] =
              !(vary > 0.0 && std::isfinite(vary) && std::isfinite(cov)) ? null_price : cov / vary;
        }
        return;
    }

    m_discard = startPos + n - 1;
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
    value_t ex = 0.0, ey = 0.0, exy = 0.0, ey2 = 0.0;
    size_t valid = 0;
    if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
        valid = 1;

    auto* dst = this->data();
    size_t first_end = startPos + n >= total ? total : startPos + n;

    for (size_t i = startPos + 1; i < first_end; i++) {
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            ey2 += iy * iy;
            exy += ix * iy;
            valid++;
        }
    }

    auto emit = [&](size_t i) {
        if (valid < 2)
            return;
        value_t nv = (value_t)valid;
        value_t vary = ey2 - ey * ey / nv;
        value_t cov = exy - ex * ey / nv;
        dst[i] =
          !(vary > 0.0 && std::isfinite(vary) && std::isfinite(cov)) ? null_price : cov / vary;
    };
    emit(first_end - 1);

    for (size_t i = first_end; i < total; i++) {
        if (std::isfinite(datax[i - n]) && std::isfinite(datay[i - n])) {
            value_t preix = datax[i - n] - kx;
            value_t preiy = datay[i - n] - ky;
            ex -= preix;
            ey -= preiy;
            ey2 -= preiy * preiy;
            exy -= preix * preiy;
            valid--;
        }
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            ey2 += iy * iy;
            exy += ix * iy;
            valid++;
        }
        emit(i);
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
    value_t null_price = Null<price_t>();

    auto const* datax = ind.data();
    auto const* datay = ref.data();
    value_t kx = datax[startPos];
    value_t ky = datay[startPos];
    if (!std::isfinite(kx))
        kx = 0.0;
    if (!std::isfinite(ky))
        ky = 0.0;
    value_t ex = 0.0, ey = 0.0, exy = 0.0, ey2 = 0.0;
    size_t valid = 0;
    if (std::isfinite(datax[startPos]) && std::isfinite(datay[startPos]))
        valid = 1;

    auto* dst = this->data();
    size_t first_end = start_pos;

    for (size_t i = startPos + 1; i < first_end; i++) {
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            ey2 += iy * iy;
            exy += ix * iy;
            valid++;
        }
    }

    for (size_t i = first_end; i < total; i++) {
        if (std::isfinite(datax[i - n]) && std::isfinite(datay[i - n])) {
            value_t preix = datax[i - n] - kx;
            value_t preiy = datay[i - n] - ky;
            ex -= preix;
            ey -= preiy;
            ey2 -= preiy * preiy;
            exy -= preix * preiy;
            valid--;
        }
        if (std::isfinite(datax[i]) && std::isfinite(datay[i])) {
            value_t ix = datax[i] - kx;
            value_t iy = datay[i] - ky;
            ex += ix;
            ey += iy;
            ey2 += iy * iy;
            exy += ix * iy;
            valid++;
        }
        if (valid < 2)
            continue;
        value_t nv = (value_t)valid;
        value_t vary = ey2 - ey * ey / nv;
        value_t cov = exy - ex * ey / nv;
        dst[i] =
          !(vary > 0.0 && std::isfinite(vary) && std::isfinite(cov)) ? null_price : cov / vary;
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
