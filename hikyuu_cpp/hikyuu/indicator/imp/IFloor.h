/*
 * IFloor.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IFLOOR_H_
#define INDICATOR_IMP_IFLOOR_H_

#include "../Indicator.h"

namespace hku {

/**
 * Round down (round in the direction of decreasing value) to an integer
 */
class IFloor : public IndicatorImp {
    INDICATOR_IMP(IFloor)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFloor();
    virtual ~IFloor() override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IFLOOR_H_ */
