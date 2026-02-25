/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IFinance : public IndicatorImp {
    INDICATOR_IMP(IFinance)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFinance();
    virtual ~IFinance() override = default;
};

}  // namespace hku