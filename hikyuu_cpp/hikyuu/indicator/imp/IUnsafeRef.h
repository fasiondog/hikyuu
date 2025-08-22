/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IUnsafeRef : public IndicatorImp {
    INDICATOR_IMP(IUnsafeRef)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IUnsafeRef();
    virtual ~IUnsafeRef();
};

} /* namespace hku */
