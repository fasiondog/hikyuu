/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IMddCurrent : public IndicatorImp {
    INDICATOR_IMP(IMddCurrent)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMddCurrent();
    virtual ~IMddCurrent() override;
};

}  // namespace hku