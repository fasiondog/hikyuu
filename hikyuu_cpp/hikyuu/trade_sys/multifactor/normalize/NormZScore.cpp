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
    setParam<bool>("out-extreme", false);  // Remove the outliers
    setParam<bool>("recursive", false);    // Remove the outliers recursively
}

NormZScore::NormZScore(bool outExtreme, double nsigma, bool recursive)
: NormalizeBase("NORM_Zscore") {
    setParam<double>("nsigma", nsigma);
    setParam<bool>("out-extreme", outExtreme);
    setParam<bool>("recursive", recursive);
}

NormZScore::~NormZScore() {}

void NormZScore::_checkParam(const string& name) const {
    if ("nsigma" == name) {
        HKU_ASSERT(getParam<double>("nsigma") > 0.);
    }
}

namespace {

// A full z-score + optional extreme clipping pass, recomputing mean/sigma from data
PriceList normalizeOnce(const PriceList& data, bool outExtreme, double nsigma) {
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
                } else if (ret[i] < llimit) {
                    ret[i] = llimit;
                }
            }
        }
    }

    return ret;
}

// Treat two results as equal when every position is equal, counting nan == nan as equal
bool normalizeStable(const PriceList& a, const PriceList& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++) {
        bool an = std::isnan(a[i]);
        bool bn = std::isnan(b[i]);
        if (an || bn) {
            if (an != bn) {
                return false;
            }
            continue;
        }
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

constexpr size_t MAX_RECURSION_DEPTH = 10;

}  // namespace

PriceList NormZScore::normalize(const PriceList& data) {
    size_t total = data.size();
    if (total <= 1) {
        PriceList ret(total, Null<price_t>());
        std::copy(data.begin(), data.end(), ret.begin());
        return ret;
    }

    bool outExtreme = getParam<bool>("out-extreme");
    double nsigma = getParam<double>("nsigma");
    bool recursive = getParam<bool>("recursive");

    PriceList cur = normalizeOnce(data, outExtreme, nsigma);
    if (!recursive) {
        return cur;
    }

    // Recurse only while a pass still changes the result; the depth cap bounds the loop even when
    // the clipped outliers keep re-normalizing (a fixed point need not exist)
    for (size_t depth = 0; depth < MAX_RECURSION_DEPTH; depth++) {
        PriceList next = normalizeOnce(cur, outExtreme, nsigma);
        if (normalizeStable(next, cur)) {
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