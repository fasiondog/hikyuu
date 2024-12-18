/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#pragma once

#include "ta-lib/ta_func.h"
#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TCmo : public IndicatorImp {
    INDICATOR_IMP(TCmo)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TCmo();
    virtual ~TCmo();
};

}  // namespace hku