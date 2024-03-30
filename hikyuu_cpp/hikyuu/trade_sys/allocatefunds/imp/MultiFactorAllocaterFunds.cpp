/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "MultiFactorAllocaterFunds.h"

namespace hku {

MultiFactorAllocaterFunds::MultiFactorAllocaterFunds() : AllocateFundsBase("AF_Multi_factor") {}

MultiFactorAllocaterFunds::~MultiFactorAllocaterFunds() {}

SystemWeightList MultiFactorAllocaterFunds::_allocateWeight(const Datetime& date,
                                                            const SystemWeightList& se_list) {
    return se_list;
}

AFPtr HKU_API AF_MultiFactor() {
    return make_shared<MultiFactorAllocaterFunds>();
}

}  // namespace hku