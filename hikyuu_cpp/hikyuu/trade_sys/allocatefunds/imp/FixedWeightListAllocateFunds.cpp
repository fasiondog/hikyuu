/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-07
 *      Author: fasiondog
 */

#include "FixedWeightListAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedWeightListAllocateFunds)
#endif

namespace hku {

FixedWeightListAllocateFunds::FixedWeightListAllocateFunds()
: AllocateFundsBase("AF_FixedWeightList") {
    setParam<PriceList>("weights", PriceList());
    // 公共参数必须设置为 false，禁止自动调整权重
    setParam<bool>("auto_adjust_weight", false);
}

FixedWeightListAllocateFunds::FixedWeightListAllocateFunds(const PriceList& weights)
: AllocateFundsBase("AF_FixedWeightList") {
    setParam<PriceList>("weights", weights);
    // 公共参数必须设置为 false，禁止自动调整权重
    setParam<bool>("auto_adjust_weight", false);
}

FixedWeightListAllocateFunds::~FixedWeightListAllocateFunds() {}

void FixedWeightListAllocateFunds::_checkParam(const string& name) const {
    if ("auto_adjust_weight" == name) {
        bool auto_adjust_weight = getParam<bool>("auto_adjust_weight");
        HKU_CHECK(!auto_adjust_weight, R"(param "auto_adjust_weight" must be false!)");
    }
}

SystemWeightList FixedWeightListAllocateFunds ::_allocateWeight(const Datetime& date,
                                                                const SystemWeightList& se_list) {
    SystemWeightList result;
    PriceList weights = getParam<PriceList>("weights");
    size_t w_total = weights.size();
    size_t wi = 0;
    for (auto iter = se_list.begin(); iter != se_list.end() && wi < w_total; ++iter) {
        result.emplace_back(iter->sys, weights[wi++]);
    }

    return result;
}

AFPtr HKU_API AF_FixedWeightList(const PriceList& weights) {
    HKU_ERROR_IF(weights.empty(), "Input weights is empty!");
    auto p = make_shared<FixedWeightListAllocateFunds>(weights);
    p->setParam<PriceList>("weights", weights);
    return p;
}

} /* namespace hku */
