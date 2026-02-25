/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IZongGuBen : public IndicatorImp {
    INDICATOR_IMP(IZongGuBen)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IZongGuBen();
    virtual ~IZongGuBen() override;
};

} /* namespace hku */
