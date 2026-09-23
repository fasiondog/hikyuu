/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* Edge jump, jumping from less than or equal to 0.0 to > 0.0 */
class IJumpUp : public IndicatorImp {
    INDICATOR_IMP(IJumpUp)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IJumpUp();
    virtual ~IJumpUp() override;
};

} /* namespace hku */
