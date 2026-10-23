/*
 * FixedAmountAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_FixedAmount: L1 equal weight + L2 fixed amount conversion
 *  (the fixed_amount parameter).
 */

#pragma once
#ifndef FIXEDAMOUNTALLOCATEFUNDS_H_
#define FIXEDAMOUNTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The fixed amount asset allocation, every selected asset cannot exceed this amount for the trade.
 * @details The pricing lives in the base class (fixed_amount > 0 takes precedence over the weights
 *          in both the L1 quota pricing and the L2 mode A target conversion):
 *          - Mode A: the per-instrument net target takes the fixed amount;
 *          - Mode B: the fixed quota of every sub-system (written into contexts[i].quota).
 * @note There are boundary differences with master FixedAmountFunds: master asserts amount>500 and stops when the remaining cash < 0.6x amount
 *       while the new system uses >=0 and rebalances by the net amount.
 * @param fixed_amount the specified fixed trade amount (the default 20000.0 of this class
 *                     overrides the base class default 0.0, where 0.0 means weight pricing)
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
