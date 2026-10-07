/*
 * EqualWeightAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The semantic carrier of AF_EqualWeight: L1 equal weight 1/N (no virtual function is overridden,
 *  all the base class default implementations are used).
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#pragma once
#ifndef EQUALWEIGHTALLOCATEFUNDS_H_
#define EQUALWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * The equal weight asset allocation, it allocates the selected assets in equal proportions.
 * @details L1 defaults to the equal weight 1/N; when the parameter weight-list is not empty the fixed weights are used instead (after normalization).
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
