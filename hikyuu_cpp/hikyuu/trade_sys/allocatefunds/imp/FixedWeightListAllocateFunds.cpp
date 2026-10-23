/*
 * FixedWeightListAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "FixedWeightListAllocateFunds.h"

#include <algorithm>
#include <cmath>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedWeightListAllocateFunds)
#endif

namespace hku {

FixedWeightListAllocateFunds::FixedWeightListAllocateFunds()
: AllocateFundsBase("AF_FixedWeightList") {
    // The single-item default keeps the unconfigured instance on the equal-weight path: a length
    // mismatch falls back to equal weights anyway
    setParam<PriceList>("weights", PriceList{1.0});
}

FixedWeightListAllocateFunds::~FixedWeightListAllocateFunds() {}

void FixedWeightListAllocateFunds::_checkParam(const string& name) const {
    if ("weights" == name) {
        PriceList ws = getParam<PriceList>("weights");
        double sum = 0.0;
        for (auto w : ws) {
            HKU_ASSERT(std::isfinite(w) && w >= 0.0);
            sum += w;
        }
        // An empty list cannot price anything; an all-zero one would clear the holdings, and an
        // overflowing sum normalizes every weight to 0 or poisons the quota
        HKU_ASSERT(!ws.empty() && std::isfinite(sum) && sum > 0.0);
    }
}

AllocateFundsBase::Weights FixedWeightListAllocateFunds::_allocate(const Datetime& date,
                                                                   const TradeManagerPtr& tm,
                                                                   SubSystemContextList& contexts,
                                                                   const KQuery& query,
                                                                   const string& mode) {
    PriceList ws = getParam<PriceList>("weights");
    auto equal_weight = [&]() {
        return _applyWeights(
          date, tm, contexts, query,
          std::vector<double>(contexts.size(), 1.0 / std::max<size_t>(contexts.size(), 1)), mode);
    };
    if (ws.empty()) {
        // The factory and set_param reject the empty list: only a deserialized archive can carry
        // it, the equal weight fallback keeps the run alive but the archived state stays invalid
        HKU_WARN("{}: empty weights list (invalid state), fallback to equal weight!", name());
        return equal_weight();
    }
    if (ws.size() != contexts.size()) {
        HKU_WARN("weights size({}) != subsystems({}), fallback to equal weight! [{}]", ws.size(),
                 contexts.size(), name());
        return equal_weight();
    }
    // Without normalization: take the fixed proportions one by one in order
    return _applyWeights(date, tm, contexts, query, std::vector<double>(ws.begin(), ws.end()),
                         mode);
}

}  // namespace hku
