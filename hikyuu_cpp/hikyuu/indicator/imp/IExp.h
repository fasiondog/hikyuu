/*
 * IExp.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IEXP_H_
#define INDICATOR_IMP_IEXP_H_

#include "../Indicator.h"

namespace hku {

class IExp : public IndicatorImp {
    INDICATOR_IMP(IExp)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IExp();
    virtual ~IExp();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IEXP_H_ */
