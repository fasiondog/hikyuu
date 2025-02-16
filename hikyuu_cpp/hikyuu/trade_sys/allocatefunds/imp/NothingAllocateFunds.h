/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-17
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_IMP_NOTHINGALLOCATEFUNDS_H_
#define TRADE_SYS_ALLOCATEFUNDS_IMP_NOTHINGALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

class NothingAllocateFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(NothingAllocateFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NothingAllocateFunds();
    virtual ~NothingAllocateFunds();
};

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_IMP_NOTHINGALLOCATEFUNDS_H_ */
