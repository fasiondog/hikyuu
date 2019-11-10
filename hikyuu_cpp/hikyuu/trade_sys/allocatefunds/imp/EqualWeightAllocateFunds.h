/*
 * EqualWeightAllocateFunds.h
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_IMP_EQUALWEIGHTALLOCATEFUNDS_H_
#define TRADE_SYS_ALLOCATEFUNDS_IMP_EQUALWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

class EqualWeightAllocateFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(EqualWeightAllocateFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    EqualWeightAllocateFunds();
    virtual ~EqualWeightAllocateFunds();
};

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_IMP_EQUALWEIGHTALLOCATEFUNDS_H_ */
