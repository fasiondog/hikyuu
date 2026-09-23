/*
 * MultiFactorAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_MultiFactor: L1 takes SubSystemContext::score as the weight.
 */

#pragma once
#ifndef MULTIFACTORALLOCATEFUNDS_H_
#define MULTIFACTORALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The multi-factor scoring weight asset allocation, i.e. directly taking the scores returned by SE as the weights.
 * @details The L1 system-level allocation takes SubSystemContext::score (the SE score backfilled by MultiSystem) as the weight, without normalization.
 *          When all the scores are 0 it falls back to the equal weight, to avoid the zero weight causing the failure to allocate.
 * @note It depends on MultiSystem backfilling score with the result of SelectorBase::getSelected() before L1
 *       (see design.md §5.3 / O2).
 * @ingroup AllocateFunds
 */
class MultiFactorAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(MultiFactorAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MultiFactorAllocateFunds();
    virtual ~MultiFactorAllocateFunds();

    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query) override;
};

}  // namespace hku

#endif /* MULTIFACTORALLOCATEFUNDS_H_ */
