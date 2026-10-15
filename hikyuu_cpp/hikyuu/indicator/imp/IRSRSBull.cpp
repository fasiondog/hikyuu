/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include <climits>
#include "IRSRSBull.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRSRSBull)
#endif

namespace hku {

IRSRSBull::IRSRSBull() : IndicatorImp("RSRS_BULL", 4) {
    m_need_context = true;
    setParam<int>("n", 20);  // Regression window
    setParam<int>("m", 60);  // Z-score window
}

IRSRSBull::~IRSRSBull() {}

void IRSRSBull::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2 && getParam<int>("n") < INT_MAX);
    } else if ("m" == name) {
        HKU_ASSERT(getParam<int>("m") >= 2 && getParam<int>("m") < INT_MAX);
    }
}

void IRSRSBull::_calculate(const Indicator&) {
    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    int m = getParam<int>("m");

    _readyBuffer(total, 4);

    size_t start_idx = n - 1;
    size_t z_start = start_idx + m - 1;

    if (total <= z_start) {
        m_discard = total;
        return;
    }

    auto* bull = this->data(0);  // The level 4 correction value (the right skew correction)
    auto* beta = this->data(1);  // The beta value
    auto* r2 = this->data(2);    // The R2 value
    auto* z = this->data(3);     // The Z-score value
    value_t null_price = Null<price_t>();

    // Centered accumulation with a valid_count filter (ST-034/ST-N03 contract)
    value_t kx = k[0].lowPrice;
    value_t ky = k[0].highPrice;
    if (!std::isfinite(kx))
        kx = 0.0;
    if (!std::isfinite(ky))
        ky = 0.0;

    // Step 1: calculate the beta and the R2 of every point (starting from start_idx)
    value_t ex = 0.0, ey = 0.0, exx = 0.0, eyy = 0.0, exy = 0.0;
    size_t valid = 0;
    for (size_t i = 0; i < static_cast<size_t>(n); i++) {
        value_t x = k[i].lowPrice;
        value_t y = k[i].highPrice;
        if (std::isfinite(x) && std::isfinite(y)) {
            value_t ix = x - kx;
            value_t iy = y - ky;
            ex += ix;
            ey += iy;
            exx += ix * ix;
            eyy += iy * iy;
            exy += ix * iy;
            valid++;
        }
    }

    auto emit_beta_r2 = [&](size_t i) {
        if (valid < 2)
            return;
        value_t nv = (value_t)valid;
        value_t varx = exx - ex * ex / nv;
        value_t vary = eyy - ey * ey / nv;
        value_t cov = exy - ex * ey / nv;
        if (!(varx > 0.0 && vary > 0.0 && std::isfinite(varx) && std::isfinite(vary) &&
              std::isfinite(cov))) {
            return;
        }
        beta[i] = cov / varx;
        // R2 = beta^2 * varx / vary (SSR/SST under the same sample count)
        r2[i] = (beta[i] * beta[i]) * varx / vary;
    };
    emit_beta_r2(start_idx);

    for (size_t i = n; i < total; i++) {
        value_t old_x = k[i - n].lowPrice;
        value_t old_y = k[i - n].highPrice;
        if (std::isfinite(old_x) && std::isfinite(old_y)) {
            value_t pix = old_x - kx;
            value_t piy = old_y - ky;
            ex -= pix;
            ey -= piy;
            exx -= pix * pix;
            eyy -= piy * piy;
            exy -= pix * piy;
            valid--;
        }
        value_t new_x = k[i].lowPrice;
        value_t new_y = k[i].highPrice;
        if (std::isfinite(new_x) && std::isfinite(new_y)) {
            value_t pix = new_x - kx;
            value_t piy = new_y - ky;
            ex += pix;
            ey += piy;
            exx += pix * pix;
            eyy += piy * piy;
            exy += pix * piy;
            valid++;
        }
        emit_beta_r2(i);
    }

    // Step 2: calculate the Z-score of the rolling M days
    // Each window judges its own valid betas: a first window without enough of them must not skip
    // every following one
    value_t sum_beta = 0.0, sum_beta2 = 0.0;
    size_t valid_count = 0;
    for (size_t i = start_idx; i <= z_start; i++) {
        if (std::isfinite(beta[i])) {
            sum_beta += beta[i];
            sum_beta2 += beta[i] * beta[i];
            valid_count++;
        }
    }

    auto emit_z_bull = [&](size_t i) {
        if (valid_count < 2 || !std::isfinite(beta[i]) || !std::isfinite(r2[i]))
            return;
        value_t nv = (value_t)valid_count;
        value_t mean_beta = sum_beta / nv;
        // clamp to 0.0 to guard against the tiny negative caused by float rounding on a
        // near-constant beta sequence (ST-014 pattern)
        value_t var_beta =
          std::max<value_t>(0.0, (sum_beta2 - sum_beta * sum_beta / nv) / (nv - 1.0));
        value_t std_beta = std::sqrt(var_beta);
        if (std_beta > 0.0) {
            z[i] = (beta[i] - mean_beta) / std_beta;
            bull[i] = z[i] * r2[i] * beta[i];
        }
    };
    emit_z_bull(z_start);

    // Calculate the following Z-scores and the level 4 correction values in a rolling way
    for (size_t i = z_start + 1; i < total; i++) {
        // Remove the oldest beta
        size_t oldest_idx = i - m;
        if (oldest_idx >= start_idx && std::isfinite(beta[oldest_idx])) {
            sum_beta -= beta[oldest_idx];
            sum_beta2 -= beta[oldest_idx] * beta[oldest_idx];
            valid_count--;
        }

        // Add the new beta
        if (std::isfinite(beta[i])) {
            sum_beta += beta[i];
            sum_beta2 += beta[i] * beta[i];
            valid_count++;
        }

        emit_z_bull(i);
    }

    // Set the discard and set the positions before beta and r2 to Null (refer to the ADX way)
    m_discard = z_start;
    for (size_t i = 0; i < z_start; i++) {
        beta[i] = null_price;
        r2[i] = null_price;
    }
}

Indicator HKU_API RSRS_BULL(int n, int m) {
    auto p = make_shared<IRSRSBull>();
    p->setParam<int>("n", n);
    p->setParam<int>("m", m);
    return Indicator(p);
}

Indicator HKU_API RSRS_BULL(const KData& kdata, int n, int m) {
    auto p = make_shared<IRSRSBull>();
    p->setParam<int>("n", n);
    p->setParam<int>("m", m);
    p->setContext(kdata);
    return Indicator(p);
}

}  // namespace hku
