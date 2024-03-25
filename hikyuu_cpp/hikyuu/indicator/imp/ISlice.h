/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class ISlice : public IndicatorImp {
    INDICATOR_IMP(ISlice)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISlice();
    ISlice(const PriceList&, int64_t start, int64_t end);
    virtual ~ISlice();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku