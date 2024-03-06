/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IMrr : public IndicatorImp {
    INDICATOR_IMP(IMrr)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMrr();
    virtual ~IMrr();
};

}  // namespace hku