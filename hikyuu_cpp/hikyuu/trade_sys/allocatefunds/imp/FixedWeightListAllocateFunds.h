/*
 * FixedWeightListAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_FixedWeightList: L1 takes the fixed proportion list in order (without normalization).
 */

#pragma once
#ifndef FIXEDWEIGHTLISTALLOCATEFUNDS_H_
#define FIXEDWEIGHTLISTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The fixed proportion list asset allocation.
 * @details The L1 system-level allocation **takes weights[i] one by one in the sub-system order, without normalization**; when the quantity does not match it falls back to the equal weight.
 *          Equivalent to master FixedWeightListAllocateFunds with auto_adjust_weight=false.
 * @param weights the fixed proportion list of every sub-system
 * @ingroup AllocateFunds
 */
class FixedWeightListAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(FixedWeightListAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedWeightListAllocateFunds();
    virtual ~FixedWeightListAllocateFunds();

    virtual void _checkParam(const string& name) const override;

    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query) override;
};

}  // namespace hku

#endif /* FIXEDWEIGHTLISTALLOCATEFUNDS_H_ */
