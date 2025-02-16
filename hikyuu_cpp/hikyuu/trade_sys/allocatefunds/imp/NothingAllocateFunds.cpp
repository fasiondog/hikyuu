/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-17
 *      Author: fasiondog
 */

#include "NothingAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NothingAllocateFunds)
#endif

namespace hku {

NothingAllocateFunds::NothingAllocateFunds() : AllocateFundsBase("AF_Nothing") {}

NothingAllocateFunds::~NothingAllocateFunds() {}

SystemWeightList NothingAllocateFunds ::_allocateWeight(const Datetime& date,
                                                        const SystemWeightList& se_list) {
    return SystemWeightList();
}

AFPtr HKU_API AF_Nothing() {
    return make_shared<NothingAllocateFunds>();
}

} /* namespace hku */
