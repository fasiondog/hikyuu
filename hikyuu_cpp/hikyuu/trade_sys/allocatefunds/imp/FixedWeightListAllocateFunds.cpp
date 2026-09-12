/*
 * FixedWeightListAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "FixedWeightListAllocateFunds.h"

#include <algorithm>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedWeightListAllocateFunds)
#endif

namespace hku {

FixedWeightListAllocateFunds::FixedWeightListAllocateFunds() : AllocateFundsBase("AF_FixedWeightList") {
    setParam<PriceList>("weights", PriceList());
}

FixedWeightListAllocateFunds::~FixedWeightListAllocateFunds() {}

void FixedWeightListAllocateFunds::_checkParam(const string& name) const {
    if ("weights" == name) {
        PriceList ws = getParam<PriceList>("weights");
        for (auto w : ws) {
            HKU_ASSERT(w >= 0.0);
        }
    }
}

AllocateFundsBase::Weights FixedWeightListAllocateFunds::_allocate(const Datetime& date,
                                                                   const TradeManagerPtr& tm,
                                                                   SubSystemContextList& contexts,
                                                                   const KQuery& query) {
    PriceList ws = getParam<PriceList>("weights");
    if (ws.size() != contexts.size()) {
        HKU_WARN_IF(!ws.empty(),
                    "weights size({}) != subsystems({}), fallback to equal weight! [{}]", ws.size(),
                    contexts.size(), name());
        return _applyWeights(
          date, tm, contexts, query,
          std::vector<double>(contexts.size(), 1.0 / std::max<size_t>(contexts.size(), 1)));
    }
    // 不归一化：按序逐一取固定比例
    return _applyWeights(date, tm, contexts, query, std::vector<double>(ws.begin(), ws.end()));
}

}  // namespace hku
