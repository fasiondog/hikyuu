/*
 * FixedWeightAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
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
    // 不归一化：每个选中子系统都只占父总资产的固定比例（等价 master auto_adjust_weight=false）
    return _applyWeights(date, tm, contexts, query, std::vector<double>(contexts.size(), weight));
}

}  // namespace hku
