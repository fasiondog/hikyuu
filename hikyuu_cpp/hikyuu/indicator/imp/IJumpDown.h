/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* 边缘跳变，从大于0.0，跳变到 <= 0.0 */
class IJumpDown : public IndicatorImp {
    INDICATOR_IMP(IJumpDown)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IJumpDown();
    virtual ~IJumpDown();
};

} /* namespace hku */
