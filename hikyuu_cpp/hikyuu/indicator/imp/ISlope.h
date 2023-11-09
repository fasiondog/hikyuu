/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISLOPE_H
#define INDICATOR_IMP_ISLOPE_H

#include "../Indicator.h"

namespace hku {

class ISlope : public IndicatorImp {
    INDICATOR_IMP(ISlope)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISlope();
    virtual ~ISlope();
};

}  // namespace hku

#endif