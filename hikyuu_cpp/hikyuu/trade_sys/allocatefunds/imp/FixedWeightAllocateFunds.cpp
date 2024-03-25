/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
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
        double weight = getParam<double>("weight");
        HKU_ASSERT(weight > 0.0 && weight <= 1.);
    }
}

SystemWeightList FixedWeightAllocateFunds ::_allocateWeight(const Datetime& date,
                                                            const SystemWeightList& se_list) {
    SystemWeightList result;
    price_t weight = getParam<double>("weight");
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        result.emplace_back(iter->sys, weight);
    }

    return result;
}

AFPtr HKU_API AF_FixedWeight(double weight) {
    HKU_CHECK_THROW(weight > 0.0 && weight <= 1.0, std::out_of_range,
                    "input weigth ({}) is out of range [0, 1]!", weight);
    auto p = make_shared<FixedWeightAllocateFunds>();
    p->setParam<double>("weight", weight);
    return p;
}

} /* namespace hku */
