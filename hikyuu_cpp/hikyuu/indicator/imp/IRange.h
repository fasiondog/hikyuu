/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IRange : public IndicatorImp {
    INDICATOR_IMP(IRange)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRange();
    IRange(const PriceList&, int64_t start, int64_t end);
    virtual ~IRange();
};

}  // namespace hku