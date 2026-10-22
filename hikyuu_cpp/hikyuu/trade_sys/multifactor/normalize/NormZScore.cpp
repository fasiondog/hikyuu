/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormZScore.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormZScore)
#endif

namespace hku {

NormZScore::NormZScore() : NormalizeBase("NORM_Zscore") {
    setParam<double>("nsigma", 3);         // The multiple used to judge the outliers
    setParam<bool>("out_extreme", false);  // Remove the outliers
    setParam<bool>("recursive", false);    // Remove the outliers recursively
}

NormZScore::NormZScore(bool outExtreme, double nsigma, bool recursive)
: NormalizeBase("NORM_Zscore") {
    setParam<double>("nsigma", nsigma);
    setParam<bool>("out_extreme", outExtreme);
    setParam<bool>("recursive", recursive);
}

NormZScore::~NormZScore() {}

void NormZScore::_checkParam(const string& name) const {
    HKU_CHECK(name != "out-extreme",
              "Deprecated parameter name 'out-extreme'! use 'out_extreme' instead");
    if ("nsigma" == name) {
        HKU_ASSERT(getParam<double>("nsigma") > 0.);
    }
}

namespace {

// A full z-score + optional extreme clipping pass, recomputing mean/sigma from data.
// clipped is set to true when at least one value is cut to the +/- nsigma limit.
PriceList normalizeOnce(const PriceList& data, bool outExtreme, double nsigma, bool* clipped) {
    if (clipped) {
        *clipped = false;
    }
    size_t total = data.size();
    PriceList ret(total, Null<price_t>());

    price_t sum = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            sum += data[i];
            count++;
        }
    }

    if (count <= 1) {
        // Fewer than two valid points is a degenerate cross-section (no dispersion): give the valid
        // position the neutral 0 instead of leaking its raw magnitude, consistent with the sigma
        // guard below and with NormMinMax / NormQuantile
        for (size_t i = 0; i < total; i++) {
            if (!std::isnan(data[i])) {
                ret[i] = 0.0;
            }
        }
        return ret;
    }

    price_t mean = sum / count;
    sum = 0.0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            price_t diff = data[i] - mean;
            sum += diff * diff;
        }
    }

    price_t sigma = std::sqrt(sum / (count - 1));
    // A zero-variance cross-section has no dispersion to scale; the z-score convention maps every
    // value to the neutral 0 (SPSS / statsmodels: std == 0 -> all 0) instead of the 0 / 0 nan
    bool zero_sigma = !(sigma > 0.0) || !std::isfinite(sigma);
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            ret[i] = zero_sigma ? 0.0 : (data[i] - mean) / sigma;
        }
    }

    if (outExtreme) {
        price_t ulimit = nsigma;
        price_t llimit = -nsigma;
        for (size_t i = 0; i < total; i++) {
            if (!std::isnan(ret[i])) {
                if (ret[i] > ulimit) {
                    ret[i] = ulimit;
                    if (clipped) {
                        *clipped = true;
                    }
                } else if (ret[i] < llimit) {
                    ret[i] = llimit;
                    if (clipped) {
                        *clipped = true;
                    }
                }
            }
        }
    }

    return ret;
}

constexpr size_t MAX_RECURSION_DEPTH = 10;

}  // namespace

PriceList NormZScore::normalize(const PriceList& data) {
    size_t total = data.size();
    if (total <= 1) {
        PriceList ret(total, Null<price_t>());
        for (size_t i = 0; i < total; i++) {
            if (!std::isnan(data[i])) {
                ret[i] = 0.0;
            }
        }
        return ret;
    }

    bool outExtreme = getParam<bool>("out_extreme");
    double nsigma = getParam<double>("nsigma");
    bool recursive = getParam<bool>("recursive");

    PriceList cur = normalizeOnce(data, outExtreme, nsigma, nullptr);
    // Recursion only has meaning when the outliers are being clipped
    if (!recursive || !outExtreme) {
        return cur;
    }

    // Keep re-normalizing while a pass actually clips at least one value; the depth cap bounds the
    // loop because a fixed point need not exist
    for (size_t depth = 0; depth < MAX_RECURSION_DEPTH; depth++) {
        bool clipped = false;
        PriceList next = normalizeOnce(cur, outExtreme, nsigma, &clipped);
        if (!clipped) {
            break;
        }
        cur = std::move(next);
    }
    return cur;
}

NormPtr HKU_API NORM_Zscore(bool outExtreme, double nsigma, bool recursive) {
    return std::make_shared<NormZScore>(outExtreme, nsigma, recursive);
}

}  // namespace hku