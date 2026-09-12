/*
 * MultiFactorAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF_MultiFactor 的语义载体：L1 以 SubSystemContext::score 为权重。
 */

#pragma once
#ifndef MULTIFACTORALLOCATEFUNDS_H_
#define MULTIFACTORALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 多因子评分权重的资产分配，即直接以 SE 返回的评分作为权重。
 * @details L1 系统级分配以 SubSystemContext::score（MultiSystem 回填的 SE 得分）为权重，不归一化。
 *          全部得分为 0 时回退等权，避免零权重导致无法分配。
 * @note 依赖 MultiSystem 在 L1 前用 SelectorBase::getSelected() 结果回填 score
 *       （见 design.md §5.3 / O2）。
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
