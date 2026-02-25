/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IIsLimitDown : public IndicatorImp {
    INDICATOR_IMP(IIsLimitDown)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsLimitDown();
    virtual ~IIsLimitDown() override;
};

} /* namespace hku */