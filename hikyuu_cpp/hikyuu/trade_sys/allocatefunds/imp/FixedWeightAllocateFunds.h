/*
 * FixedWeightAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF_FixedWeight 的语义载体：L1 固定比例（不归一化）。
 */

#pragma once
#ifndef FIXEDWEIGHTALLOCATEFUNDS_H_
#define FIXEDWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 固定比例资产分配，每个选中的资产都只占总资产固定的比例。
 * @details L1 系统级分配对每个选中子系统**直接返回固定 weight，不归一化**
 *          （等价 master FixedWeightAllocateFunds 的 auto_adjust_weight=false）。
 *          不使用 weight-list 参数，避免被归一化。
 * @param weight 每个子系统占父总资产的比例 (0, 1]
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
