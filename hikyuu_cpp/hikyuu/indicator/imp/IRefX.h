/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IRefX : public IndicatorImp {
    INDICATOR_IMP(IRefX)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRefX();
    virtual ~IRefX();
};

} /* namespace hku */
