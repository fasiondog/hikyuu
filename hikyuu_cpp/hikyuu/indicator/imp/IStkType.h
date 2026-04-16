/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IStkType : public IndicatorImp {
    INDICATOR_IMP(IStkType)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IStkType();
    virtual ~IStkType() override;
};

} /* namespace hku */