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

MultiFactorAllocateFunds::MultiFactorAllocateFunds() : AllocateFundsBase("AF_MultiFactor") {
    // The scores are treated as the mutual ratios of the sub-systems and normalized to sum 1 by
    // default (the 2.8.3 auto_adjust_weight semantics, so the allocation is full-scale whatever
    // the raw score range is); when false the score itself is taken as the final weight and the
    // over-quota part relies on max_total_position to scale down
    setParam<bool>("auto_adjust_weight", true);
}

MultiFactorAllocateFunds::~MultiFactorAllocateFunds() {}

AllocateFundsBase::Weights MultiFactorAllocateFunds::_allocate(const Datetime& date,
                                                               const TradeManagerPtr& tm,
                                                               SubSystemContextList& contexts,
                                                               const KQuery& query,
                                                               const string& mode) {
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
        // equal weight, to avoid the zero weight causing the failure to allocate (notified once
        // per instance: it is a normal state on the non-rebalancing days)
        if (!m_warned_zero_score) {
            m_warned_zero_score = true;
            HKU_WARN("AF_MultiFactor: all SE score is zero/absent, fallback to equal weight! [{}]",
                     name());
        }
        if (!contexts.empty()) {
            weights.assign(contexts.size(), 1.0 / contexts.size());
        }
    } else if (getParam<bool>("auto_adjust_weight")) {
        // Normalize to sum 1: the relative ratios are kept, the total allocation becomes
        // full-scale (when the score sum is less than 1 the master 2.8.3 also allocated fully)
        for (auto& w : weights) {
            w /= sum;
        }
    }
    return _applyWeights(date, tm, contexts, query, weights, mode);
}

}  // namespace hku
