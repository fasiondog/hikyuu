/*
 * FixedAmountAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_FixedAmount: L1 equal weight + L2 fixed amount conversion (the fixed-amount parameter).
 */

#pragma once
#ifndef FIXEDAMOUNTALLOCATEFUNDS_H_
#define FIXEDAMOUNTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The fixed amount asset allocation, every selected asset cannot exceed this amount for the trade.
 * @details L1 uses the base class default equal weight; L2 reads the fixed-amount parameter:
 *          - Mode A: the target market value of every suggestion takes the fixed amount;
 *          - Mode B: the fixed quota of every sub-system (written into contexts[i].quota).
 * @note There are boundary differences with master FixedAmountFunds: master asserts amount>500 and stops when the remaining cash < 0.6x amount
 *       while the new system uses >=0 and rebalances by the net amount.
 * @param fixed-amount the specified fixed trade amount
 * @ingroup AllocateFunds
 */
class FixedAmountAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(FixedAmountAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedAmountAllocateFunds();
    virtual ~FixedAmountAllocateFunds();
};

}  // namespace hku

#endif /* FIXEDAMOUNTALLOCATEFUNDS_H_ */
