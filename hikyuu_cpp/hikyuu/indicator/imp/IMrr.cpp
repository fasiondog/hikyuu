/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include <climits>
#include "IMrr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMrr)
#endif

namespace hku {

namespace {
// One-pass accumulator for MRR over a window [window_left..i]: run_min is the accumulated
// minimum of the valid points, max_rr is the maximum of src[j]/run_min_j - 1 within the window.
// NaN and non-positive values are skipped uniformly across the static warmup, the full-window
// branch and the incremental scan (ST-010: the three paths previously diverged on how invalid
// inputs were treated).
struct MrrAccum {
    Indicator::value_t run_min = 0.0;
    Indicator::value_t max_rr = 0.0;
    bool has_valid = false;

    void feed(Indicator::value_t v) {
        if (!std::isfinite(v) || v <= 0.0) {
            return;
        }
        if (!has_valid) {
            run_min = v;
            has_valid = true;
            return;
        }
        if (v < run_min) {
            run_min = v;
        }
        Indicator::value_t rr = v / run_min - 1.0;
        if (rr > max_rr) {
            max_rr = rr;
        }
    }

    Indicator::value_t result() const {
        return max_rr * 100.0;
    }
};
}  // namespace

IMrr::IMrr() : IndicatorImp("MRR", 1) {
    setParam<int>("n", 0);
}

IMrr::~IMrr() {}

void IMrr::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0 && getParam<int>("n") < INT_MAX);
    }
}

void IMrr::_calculate(const Indicator& ind) {
    m_discard = ind.discard();
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard >= total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t n = static_cast<size_t>(getParam<int>("n"));
    if (n == 0 || n > total - m_discard) {
        n = total - m_discard;
    }

    if (n == 1) {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = (!std::isfinite(src[i]) || src[i] <= 0.0) ? Null<value_t>() : 0.0;
        }
        return;
    }

    // Warmup (window_left = m_discard, growing prefix), then delegate the sliding tail to
    // _increment_calculate. When n == total - m_discard the warmup covers everything and the
    // incremental call is skipped.
    MrrAccum acc;
    size_t warmup_end = m_discard + n;
    for (size_t i = m_discard; i < warmup_end; ++i) {
        if (!std::isfinite(src[i]) || src[i] <= 0.0) {
            dst[i] = Null<value_t>();
            continue;
        }
        acc.feed(src[i]);
        dst[i] = acc.result();
    }

    if (warmup_end < total) {
        _increment_calculate(ind, warmup_end);
    }
}

bool IMrr::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMrr::min_increment_start() const {
    return getParam<int>("n");
}

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // The standard maximum rise rate semantics: rr_j = src[j] / run_min_j - 1, where
    // run_min_j = min(src[window_left..j]) is the accumulated minimum up to j.
    // The original implementation wrongly used the global minimum of the window as the rise base
    // of all the points; when the lowest point of the window appears after the highest point it
    // introduces the look-ahead bias and overestimates the rise rate.
    // Here the original O(1) fast path state machine is abandoned (its judgment of
    // current_rr > window_max_rr does not hold in principle under the standard MRR semantics)
    // and it degenerates to an O(n) brute force scan per point, using the run_min base to
    // guarantee the correctness. It is symmetric to the IMdd fix.
    for (size_t i = start_pos; i < total; ++i) {
        if (!std::isfinite(src[i]) || src[i] <= 0.0) {
            dst[i] = Null<value_t>();
            continue;
        }
        size_t window_left = i + 1 - n;
        MrrAccum acc;
        for (size_t j = window_left; j <= i; ++j) {
            acc.feed(src[j]);
        }
        if (acc.has_valid) {
            dst[i] = acc.result();
        }
    }
}

Indicator HKU_API MRR(int n) {
    IndicatorImpPtr p = make_shared<IMrr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku