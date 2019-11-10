/*
 * ILn.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILN_H_
#define INDICATOR_IMP_ILN_H_

#include "../Indicator.h"

namespace hku {

class ILn : public IndicatorImp {
    INDICATOR_IMP(ILn)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILn();
    virtual ~ILn();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILN_H_ */
