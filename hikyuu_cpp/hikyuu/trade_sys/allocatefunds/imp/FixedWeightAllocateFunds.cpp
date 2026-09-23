/*
 * FixedWeightAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#include "FixedWeightAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedWeightAllocateFunds)
#endif

namespace hku {

FixedWeightAllocateFunds::FixedWeightAllocateFunds() : AllocateFundsBase("AF_FixedWeight") {
    setParam<double>("weight", 0.1);

}

FixedWeightAllocateFunds::~FixedWeightAllocateFunds() {}

void FixedWeightAllocateFunds::_checkParam(const string& name) const {
    if ("weight" == name) {
        double w = getParam<double>("weight");
        HKU_ASSERT(w > 0.0 && w <= 1.0);
    }
}

AllocateFundsBase::Weights FixedWeightAllocateFunds::_allocate(const Datetime& date,
                                                               const TradeManagerPtr& tm,
                                                               SubSystemContextList& contexts,
                                                               const KQuery& query) {
    double weight = getParam<double>("weight");
    // Without normalization: every selected sub-system only accounts for a fixed proportion of the parent total assets (equivalent to master auto_adjust_weight=false)
    return _applyWeights(date, tm, contexts, query, std::vector<double>(contexts.size(), weight));
}

}  // namespace hku
