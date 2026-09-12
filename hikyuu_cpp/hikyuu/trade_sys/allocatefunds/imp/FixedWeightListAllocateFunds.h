/*
 * FixedWeightListAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF_FixedWeightList 的语义载体：L1 按序取固定比例列表（不归一化）。
 */

#pragma once
#ifndef FIXEDWEIGHTLISTALLOCATEFUNDS_H_
#define FIXEDWEIGHTLISTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 固定比例列表资产分配。
 * @details L1 系统级分配按子系统顺序**逐一取 weights[i]，不归一化**；数量不符时回退等权。
 *          等价 master FixedWeightListAllocateFunds 的 auto_adjust_weight=false。
 * @param weights 各子系统固定比例列表
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
