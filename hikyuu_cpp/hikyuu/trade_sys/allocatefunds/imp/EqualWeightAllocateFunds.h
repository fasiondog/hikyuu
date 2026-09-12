/*
 * EqualWeightAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF_EqualWeight 的语义载体：L1 等权 1/N（不重写任何虚函数，全部使用基类默认实现）。
 */

#pragma once
#ifndef EQUALWEIGHTALLOCATEFUNDS_H_
#define EQUALWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 等权重资产分配，对选中的资产进行等比例分配。
 * @details L1 默认等权 1/N；若参数 weight-list 非空则改用该固定权重（归一化后）。
 * @ingroup AllocateFunds
 */
class EqualWeightAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(EqualWeightAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    EqualWeightAllocateFunds();
    virtual ~EqualWeightAllocateFunds();
};

}  // namespace hku

#endif /* EQUALWEIGHTALLOCATEFUNDS_H_ */
