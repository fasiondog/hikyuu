/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IResult : public IndicatorImp {
    INDICATOR_IMP(IResult)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IResult();
    explicit IResult(int reuslt_ix);
    virtual ~IResult() = default;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku