/*
 * ISma.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#include <algorithm>
#include <climits>
#include "../crt/CVAL.h"
#include "ISma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISma)
#endif

namespace hku {

ISma::ISma() : IndicatorImp("SMA", 1) {
    setParam<int>("n", 22);
    setParam<double>("m", 2.0);
}

ISma::~ISma() {}

void ISma::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1 && getParam<int>("n") < INT_MAX);
    }

    // the recursion is unstable outside 1 <= m <= n, the calculation clamps m into that range
    if (haveParam("m")) {
        double m = getParam<double>("m");
        int n = getParam<int>("n");
        HKU_WARN_IF(m < 1.0 || m > n,
                    "SMA: m({}) is outside [1, n({})], clamped in the calculation!", m, n);
    }
}

void ISma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _set(ind[m_discard], m_discard);
    _increment_calculate(ind, m_discard);
}

size_t ISma::min_increment_start() const {
    return 1;
}

void ISma::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    double n = getParam<int>("n");
    double m = getParam<double>("m");
    if (m < 1.0 || m > n) {
        HKU_WARN("SMA: m({}) is outside [1, n({})], clamped in the calculation!", m, n);
        m = std::clamp(m, 1.0, n);
    }

    auto const* src = data.data();
    auto* dst = this->data();

    double p = n - m;
    for (size_t i = start_pos + 1; i < total; i++) {
        if (!std::isfinite(src[i])) {
            dst[i] = dst[i - 1];
        } else if (!std::isfinite(dst[i - 1])) {
            dst[i] = src[i];
        } else {
            dst[i] = (m * src[i] + p * dst[i - 1]) / n;
        }
    }
}

void ISma::_dyn_one_circle(const Indicator& ind, size_t curPos, int n, double m) {
    HKU_IF_RETURN(n < 1, void());
    // Intentional: replicate the static SMA recursion on the prefix [0, curPos] with raw scalars,
    // bitwise identical to SMA(SLICE(ind, 0, curPos + 1)) but without per-bar allocations.
    auto const* src = ind.data();
    double dn = n;
    double dm = std::clamp(m, 1.0, dn);
    double p = dn - dm;
    price_t prev = src[ind.discard()];
    for (size_t i = ind.discard() + 1; i <= curPos; ++i) {
        if (!std::isfinite(src[i])) {
            // hold prev
        } else if (!std::isfinite(prev)) {
            prev = src[i];
        } else {
            prev = (dm * src[i] + p * prev) / dn;
        }
    }
    _set(prev, curPos);
}

void ISma::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("n");
    Indicator n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n"));
    iter = m_ind_params.find("m");
    Indicator m =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<double>("m"));

    HKU_CHECK(n.size() == ind.size(), "ind_param(n).size()={}, ind.size()={}!", n.size(),
              ind.size());
    HKU_CHECK(m.size() == ind.size(), "ind_param(m).size()={}, ind.size()={}!", m.size(),
              ind.size());

    m_discard = std::max(ind.discard(), n.discard());
    m_discard = std::max(m_discard, m.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    global_parallel_for_index_void(
      ind.discard(), total, [&](size_t i) { _dyn_one_circle(ind, i, n[i], m[i]); }, 400);

    updateDiscard();
}

Indicator HKU_API SMA(int n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(int n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(const IndParam& n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(const IndParam& n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

} /* namespace hku */
