/*
 * EqualWeightAllocateFunds.cpp
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#include "EqualWeightAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::EqualWeightAllocateFunds)
#endif

namespace hku {

EqualWeightAllocateFunds::EqualWeightAllocateFunds() : AllocateFundsBase("AF_EqualWeight") {}

EqualWeightAllocateFunds::~EqualWeightAllocateFunds() {}

SystemWeightList EqualWeightAllocateFunds ::_allocateWeight(const Datetime& date,
                                                            const SystemList& se_list) {
    SystemWeightList result;
    double weight = 1.0 / se_list.size();
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        result.emplace_back(*iter, weight);
    }

    return result;
}

AFPtr HKU_API AF_EqualWeight() {
    return make_shared<EqualWeightAllocateFunds>();
}

} /* namespace hku */
