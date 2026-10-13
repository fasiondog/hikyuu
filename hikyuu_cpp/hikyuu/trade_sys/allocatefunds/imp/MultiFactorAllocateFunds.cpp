/*
 * MultiFactorAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "MultiFactorAllocateFunds.h"

#include <algorithm>
#include <cmath>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorAllocateFunds)
#endif

namespace hku {

MultiFactorAllocateFunds::MultiFactorAllocateFunds() : AllocateFundsBase("AF_MultiFactor") {}

MultiFactorAllocateFunds::~MultiFactorAllocateFunds() {}

AllocateFundsBase::Weights MultiFactorAllocateFunds::_allocate(const Datetime& date,
                                                               const TradeManagerPtr& tm,
                                                               SubSystemContextList& contexts,
                                                               const KQuery& query) {
    std::vector<double> weights(contexts.size(), 0.0);
    double sum = 0.0;
    for (size_t i = 0; i < contexts.size(); ++i) {
        double s = contexts[i].score;
        // A non-finite score (e.g. from a broken IC chain) is treated as 0 like the unselected
        if (!std::isfinite(s)) {
            s = 0.0;
        }
        if (s < 0.0) {
            s = 0.0;  // The negative scores are treated as 0 (the short quota allocation is not
                      // supported)
        }
        weights[i] = s;
        sum += s;
    }
    if (!(sum > 0.0) || !std::isfinite(sum)) {
        // SE is not configured / a non-rebalancing day / all the scores are zero: fall back to the
        // equal weight, to avoid the zero weight causing the failure to allocate
        HKU_WARN("AF_MultiFactor: all SE score is zero/absent, fallback to equal weight! [{}]",
                 name());
        if (!contexts.empty()) {
            weights.assign(contexts.size(), 1.0 / contexts.size());
        }
    }
    // Without normalization: the score is the weight (equivalent to master
    // MultiFactorAllocaterFunds returning se_list as is)
    return _applyWeights(date, tm, contexts, query, weights);
}

}  // namespace hku
