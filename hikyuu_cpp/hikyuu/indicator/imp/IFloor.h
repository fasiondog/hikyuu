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
 * 向下舍入(向数值减小方向舍入)取整
 */
class IFloor : public IndicatorImp {
    INDICATOR_IMP(IFloor)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFloor();
    virtual ~IFloor();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IFLOOR_H_ */
