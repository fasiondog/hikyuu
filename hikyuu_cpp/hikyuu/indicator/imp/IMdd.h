/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IMdd : public IndicatorImp {
    INDICATOR_IMP(IMdd)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMdd();
    virtual ~IMdd();
};

}  // namespace hku