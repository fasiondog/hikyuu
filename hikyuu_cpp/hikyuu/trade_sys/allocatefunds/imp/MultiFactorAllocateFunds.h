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
 * The multi-factor scoring weight asset allocation, taking the scores returned by SE as the
 * weights.
 * @details The L1 system-level allocation takes SubSystemContext::score (the SE score backfilled by
 * MultiSystem) as the weight. A non-finite score (NaN / inf) is treated as 0, like the unselected
 * sub-systems. When all the scores are 0 (or the sum is non-finite) it falls back to the equal
 * weight, to avoid the zero weight causing the failure to allocate.
 * @param auto_adjust_weight when true (the default) the scores are treated as the mutual ratios
 * and normalized to sum 1, so the total allocation is full-scale whatever the raw score range is
 * (the 2.8.3 semantics: a score sum less than 1 also allocates fully); when false the score itself
 * is the final weight and the part exceeding the budget relies on max_total_position to scale down.
 * @note It depends on MultiSystem backfilling score with the result of SelectorBase::getSelected()
 * before L1
 *
 * @ingroup AllocateFunds
 */
class MultiFactorAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(MultiFactorAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MultiFactorAllocateFunds();
    virtual ~MultiFactorAllocateFunds();

    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query,
                              const string& mode) override;

private:
    mutable bool m_warned_zero_score{false};
};

}  // namespace hku

#endif /* MULTIFACTORALLOCATEFUNDS_H_ */
