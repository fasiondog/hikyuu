/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  The pure function implementation of the industry neutralization, extracted as an internal inline
 *  header to support the white-box unit tests.
 *  Both MultiFactorBase.cpp and test_MF_IndustryNeutralize.cpp include this header.
 */

#pragma once

#include <cmath>
#include <vector>
#include "hikyuu/DataType.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/Null.h"

namespace hku {

// Calculate the industry neutralized factor: subtract the group mean within every industry group,
// residual = y_i - mean(y_{group(i)}).
//
// Mathematical equivalence: when the industry attribution is mutually exclusive (a stock belongs to
// one industry only) and only this one categorical variable is neutralized,
// "taking the residual of the one-hot multiple regression without the global intercept" is strictly
// equivalent to "subtracting the group mean":
//   min_{β} Σ_i (y_i - Σ_k X_{i,k} β_k)^2; since X is a mutually exclusive one-hot, it can be split
//   into the independent sums of every industry,
//   the analytical solution is β_k = mean(y_{group(k)}) and the residual is
//   e_i = y_i - mean(y_{group(i)}).
// Therefore there is no need to build the design matrix and perform the QR decomposition; two
// passes of scanning O(n) are enough, and it is naturally compatible with the daily dynamic NaN
// mask (the invalid values are skipped when calculating the group mean), avoiding the model
// misspecification of the original integer encoding + univariate regression (the result depends on
// the arrangement order of the block list) and the contention of the Eigen matrix heap allocation
// under multiple threads.
//
// labels[i] = the industry index of the stock i (0..blk_count-1), and it is blk_count if there is
// no attribution (skipped and the residual is set to NaN).
inline PriceList calculate_industry_residuals(const PriceList& y, const PriceList& labels,
                                              size_t blk_count) {
    HKU_ASSERT(y.size() == labels.size());

    const size_t n = y.size();
    PriceList residuals(n, Null<price_t>());

    // When the whole market is one industry (k=1) or there is no industry classification,
    // subtracting the mean degrades to subtracting the global mean; all NaN when there is no valid
    // data.
    if (blk_count == 0) {
        return residuals;
    }

    std::vector<double> sums(blk_count, 0.0);
    std::vector<size_t> counts(blk_count, 0);

    // Pass 1: accumulate the sum and the count of the valid y of every group (the NaN/Inf and the
    // stocks without attribution are skipped)
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        double label = labels[i];
        // Guard against NaN/Inf/negative/huge values: static_cast<size_t>(a negative value) is UB,
        // it may overflow to SIZE_MAX and cause a segfault of sums[g] out of range, so it must be
        // intercepted with a double comparison before the cast.
        if (std::isnan(label) || std::isinf(label) || label < 0.0 ||
            label >= static_cast<double>(blk_count)) {
            continue;  // No attribution or an illegal label does not participate in the group mean
        }
        size_t g = static_cast<size_t>(label);
        sums[g] += y[i];
        counts[g]++;
    }

    // Pass 2: residual = y_i - mean(y_{group(i)})
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            continue;  // Keep NaN (consistent with the semantics of the original
                       // calculate_residuals)
        }
        double label = labels[i];
        if (std::isnan(label) || std::isinf(label) || label < 0.0 ||
            label >= static_cast<double>(blk_count)) {
            continue;  // No attribution or an illegal label: the residual is set to NaN
        }
        size_t g = static_cast<size_t>(label);
        if (counts[g] == 0) {
            continue;  // The group has no valid sample: the residual is set to NaN
        }
        residuals[i] = y[i] - sums[g] / static_cast<double>(counts[g]);
    }

    return residuals;
}

}  // namespace hku
