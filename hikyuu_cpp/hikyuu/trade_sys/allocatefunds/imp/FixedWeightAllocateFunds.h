/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_IMP_FIXEDWEIGHTALLOCATEFUNDS_H_
#define TRADE_SYS_ALLOCATEFUNDS_IMP_FIXEDWEIGHTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

class FixedWeightAllocateFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(FixedWeightAllocateFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedWeightAllocateFunds();
    virtual ~FixedWeightAllocateFunds();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_IMP_FIXEDWEIGHTALLOCATEFUNDS_H_ */
