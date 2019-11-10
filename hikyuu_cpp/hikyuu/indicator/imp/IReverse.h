/*
 * IReverse.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IREVERSE_H_
#define INDICATOR_IMP_IREVERSE_H_

#include "../Indicator.h"

namespace hku {

class IReverse : public IndicatorImp {
    INDICATOR_IMP(IReverse)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IReverse();
    virtual ~IReverse();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IREVERSE_H_ */
