/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-25
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IIsLimitUp : public IndicatorImp {
    INDICATOR_IMP(IIsLimitUp)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsLimitUp();
    virtual ~IIsLimitUp() override;
};

} /* namespace hku */
