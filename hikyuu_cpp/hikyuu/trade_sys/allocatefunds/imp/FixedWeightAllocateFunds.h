/*
 * FixedWeightAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_FixedWeight: L1 fixed proportion (without normalization).
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDWEIGHTALLOCATEFUNDS_H_
#define FIXEDWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The fixed proportion asset allocation, every selected asset only accounts for a fixed proportion of the total assets.
 * @details The L1 system-level allocation **directly returns the fixed weight without normalization** for every selected sub-system
 *          (equivalent to master FixedWeightAllocateFunds with auto_adjust_weight=false).
 *          The weight-list parameter is not used, to avoid being normalized.
 * @param weight the proportion of every sub-system in the parent total assets (0, 1]
 * @ingroup AllocateFunds
 */
class FixedWeightAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(FixedWeightAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedWeightAllocateFunds();
    virtual ~FixedWeightAllocateFunds();

    virtual void _checkParam(const string& name) const override;

    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query) override;
};

}  // namespace hku

#endif /* FIXEDWEIGHTALLOCATEFUNDS_H_ */
