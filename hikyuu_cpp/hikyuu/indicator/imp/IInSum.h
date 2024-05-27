/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IINSUM_H_
#define INDICATOR_IMP_IINSUM_H_

#include "../Indicator.h"

namespace hku {

class IInSum : public IndicatorImp {
    INDICATOR_IMP(IInSum)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IInSum();
    virtual ~IInSum();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku

#endif /* INDICATOR_IMP_IINSUM_H_ */