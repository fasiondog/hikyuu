/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#include "ISlope.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISlope)
#endif

namespace hku {

ISlope::ISlope() : IndicatorImp("SLOPE", 3) {
    setParam<int>("n", 22);
}

ISlope::~ISlope() {}

void ISlope::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void ISlope::_calculate(const Indicator& ind) {
    size_t total = ind.size();

    int n = getParam<int>("n");
    m_discard = ind.discard() + (n > 1 ? (size_t)n - 1 : 1);
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst_slope = this->data(0);
    auto* dst_r2 = this->data(1);
    auto* dst_relmaxres = this->data(2);

    if (n <= 1) {
        for (size_t i = m_discard; i < total; i++) {
            dst_slope[i] = 0.0;
            dst_r2[i] = 0.0;
            dst_relmaxres[i] = 0.0;
        }
        return;
    }

    // the rolling sums start at the first valid input value, whatever the declared discard is
    size_t startPos = ind.discard();
    price_t fn = (price_t)n;
    price_t S_y = src[startPos];
    price_t S_xy = 0.0;
    price_t S_y2 = src[startPos] * src[startPos];

    // Warmup: partial windows of increasing size [startPos, i]
    size_t warmup_end = startPos + (size_t)n;
    if (warmup_end > total)
        warmup_end = total;

    for (size_t i = startPos + 1; i < warmup_end; i++) {
        size_t cnt = i - startPos + 1;
        price_t x_rel = (price_t)(cnt - 1);
        S_y += src[i];
        S_xy += x_rel * src[i];
        S_y2 += src[i] * src[i];

        price_t fcnt = (price_t)cnt;
        price_t sum_x = fcnt * (fcnt - 1.0) / 2.0;
        price_t denom = fcnt * fcnt * (fcnt * fcnt - 1.0) / 12.0;
        price_t ss_xy = fcnt * S_xy - sum_x * S_y;
        price_t ss_yy = fcnt * S_y2 - S_y * S_y;

        price_t slope = ss_xy / denom;
        if (i < m_discard) {
            continue;
        }

        dst_slope[i] = slope;
        dst_r2[i] = (ss_yy != 0.0) ? (ss_xy * ss_xy / (denom * ss_yy)) : 0.0;

        price_t x_mean = sum_x / fcnt;
        price_t y_mean = S_y / fcnt;
        price_t intercept = y_mean - slope * x_mean;
        price_t max_res = 0.0;
        for (size_t k = 0; k < cnt; k++) {
            price_t res = std::abs(src[startPos + k] - (intercept + slope * (price_t)k));
            if (res > max_res)
                max_res = res;
        }
        dst_relmaxres[i] = (y_mean != 0.0) ? (max_res / y_mean) : Null<price_t>();
    }

    // Steady-state: full window of size n, O(1) rolling for slope/r²
    price_t full_sum_x = fn * (fn - 1.0) / 2.0;
    price_t full_denom = fn * fn * (fn * fn - 1.0) / 12.0;
    price_t full_x_mean = (fn - 1.0) / 2.0;

    for (size_t i = warmup_end; i < total; i++) {
        price_t removed = src[i - n];
        price_t added = src[i];

        // O(1) rolling update for relative-x sums
        S_xy = S_xy - S_y + removed + (fn - 1.0) * added;
        S_y += added - removed;
        S_y2 += added * added - removed * removed;

        price_t ss_xy = fn * S_xy - full_sum_x * S_y;
        price_t ss_yy = fn * S_y2 - S_y * S_y;

        price_t slope = ss_xy / full_denom;
        dst_slope[i] = slope;
        dst_r2[i] = (ss_yy != 0.0) ? (ss_xy * ss_xy / (full_denom * ss_yy)) : 0.0;

        // O(n) max_residual: regression line changes each bar, exact max is inherently linear
        price_t y_mean = S_y / fn;
        price_t intercept = y_mean - slope * full_x_mean;
        size_t wstart = i - (size_t)n + 1;
        price_t max_res = 0.0;
        for (size_t k = 0; k < (size_t)n; k++) {
            price_t res = std::abs(src[wstart + k] - (intercept + slope * (price_t)k));
            if (res > max_res)
                max_res = res;
        }
        dst_relmaxres[i] = (y_mean != 0.0) ? (max_res / y_mean) : Null<price_t>();
    }
}

bool ISlope::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t ISlope::min_increment_start() const {
    return getParam<int>("n");
}

void ISlope::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst_slope = this->data(0);
    auto* dst_r2 = this->data(1);
    auto* dst_relmaxres = this->data(2);

    int n = getParam<int>("n");
    price_t fn = (price_t)n;

    // Initialize rolling sums from seed window [start_pos - n, start_pos - 1]
    // using relative x = 0, 1, ..., n-1
    price_t S_y = 0.0, S_xy = 0.0, S_y2 = 0.0;
    size_t seed_start = start_pos - (size_t)n;
    for (size_t k = 0; k < (size_t)n; k++) {
        price_t y = src[seed_start + k];
        S_y += y;
        S_xy += (price_t)k * y;
        S_y2 += y * y;
    }

    price_t full_sum_x = fn * (fn - 1.0) / 2.0;
    price_t full_denom = fn * fn * (fn * fn - 1.0) / 12.0;
    price_t full_x_mean = (fn - 1.0) / 2.0;

    for (size_t i = start_pos; i < total; i++) {
        price_t removed = src[i - n];
        price_t added = src[i];

        S_xy = S_xy - S_y + removed + (fn - 1.0) * added;
        S_y += added - removed;
        S_y2 += added * added - removed * removed;

        price_t ss_xy = fn * S_xy - full_sum_x * S_y;
        price_t ss_yy = fn * S_y2 - S_y * S_y;

        price_t slope = ss_xy / full_denom;
        dst_slope[i] = slope;
        dst_r2[i] = (ss_yy != 0.0) ? (ss_xy * ss_xy / (full_denom * ss_yy)) : 0.0;

        price_t y_mean = S_y / fn;
        price_t intercept = y_mean - slope * full_x_mean;
        size_t wstart = i - (size_t)n + 1;
        price_t max_res = 0.0;
        for (size_t k = 0; k < (size_t)n; k++) {
            price_t res = std::abs(src[wstart + k] - (intercept + slope * (price_t)k));
            if (res > max_res)
                max_res = res;
        }
        dst_relmaxres[i] = (y_mean != 0.0) ? (max_res / y_mean) : Null<price_t>();
    }
}

void ISlope::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos <= ind.discard()) {
        _set(Null<price_t>(), curPos);
        _set(Null<price_t>(), curPos, 1);
        _set(Null<price_t>(), curPos, 2);
        return;
    }

    if (step <= 1) {
        _set(0, curPos);
        _set(0, curPos, 1);
        _set(0, curPos, 2);
        return;
    }

    size_t cnt = curPos - start + 1;
    price_t fcnt = (price_t)cnt;
    price_t sum_x = fcnt * (fcnt - 1.0) / 2.0;
    price_t denom = fcnt * fcnt * (fcnt * fcnt - 1.0) / 12.0;

    price_t S_y = 0.0, S_xy = 0.0, S_y2 = 0.0;
    for (size_t k = 0; k < cnt; k++) {
        price_t y = ind[start + k];
        S_y += y;
        S_xy += (price_t)k * y;
        S_y2 += y * y;
    }

    price_t ss_xy = fcnt * S_xy - sum_x * S_y;
    price_t ss_yy = fcnt * S_y2 - S_y * S_y;

    price_t slope = ss_xy / denom;
    price_t r2 = (ss_yy != 0.0) ? (ss_xy * ss_xy / (denom * ss_yy)) : 0.0;

    price_t x_mean = sum_x / fcnt;
    price_t y_mean = S_y / fcnt;
    price_t intercept = y_mean - slope * x_mean;
    price_t max_res = 0.0;
    for (size_t k = 0; k < cnt; k++) {
        price_t res = std::abs(ind[start + k] - (intercept + slope * (price_t)k));
        if (res > max_res)
            max_res = res;
    }
    price_t relmaxres = (y_mean != 0.0) ? (max_res / y_mean) : Null<price_t>();

    _set(slope, curPos);
    _set(r2, curPos, 1);
    _set(relmaxres, curPos, 2);
}

Indicator HKU_API SLOPE(int n) {
    IndicatorImpPtr p = make_shared<ISlope>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SLOPE(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISlope>();
    p->setIndParam("n", n);
    return Indicator(p);
}

}  // namespace hku
