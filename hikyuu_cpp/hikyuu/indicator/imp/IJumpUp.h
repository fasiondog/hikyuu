/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* 边缘跳变，从小于等于0.0，跳变到 > 0.0 */
class IJumpUp : public IndicatorImp {
    INDICATOR_IMP(IJumpUp)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IJumpUp();
    virtual ~IJumpUp();
};

} /* namespace hku */
