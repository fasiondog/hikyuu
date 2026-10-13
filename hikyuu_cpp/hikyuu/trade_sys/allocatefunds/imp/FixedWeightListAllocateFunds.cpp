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
    setParam<PriceList>("weights", PriceList());
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
        // Reject all-zero weights (they would clear the holdings); an empty list falls back to
        // equal weight. An overflowing sum normalizes every weight to 0 or poisons the quota
        HKU_ASSERT(ws.empty() || (std::isfinite(sum) && sum > 0.0));
    }
}

AllocateFundsBase::Weights FixedWeightListAllocateFunds::_allocate(const Datetime& date,
                                                                   const TradeManagerPtr& tm,
                                                                   SubSystemContextList& contexts,
                                                                   const KQuery& query) {
    PriceList ws = getParam<PriceList>("weights");
    if (ws.size() != contexts.size()) {
        HKU_WARN("weights size({}) != subsystems({}), fallback to equal weight! [{}]", ws.size(),
                 contexts.size(), name());
        return _applyWeights(
          date, tm, contexts, query,
          std::vector<double>(contexts.size(), 1.0 / std::max<size_t>(contexts.size(), 1)));
    }
    // Without normalization: take the fixed proportions one by one in order
    return _applyWeights(date, tm, contexts, query, std::vector<double>(ws.begin(), ws.end()));
}

}  // namespace hku
