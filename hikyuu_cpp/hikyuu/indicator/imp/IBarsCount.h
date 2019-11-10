/*
 * IBarsCount.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSCOUNT_H_
#define INDICATOR_IMP_IBARSCOUNT_H_

#include "../Indicator.h"

namespace hku {

class IBarsCount : public IndicatorImp {
    INDICATOR_IMP(IBarsCount)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsCount();
    virtual ~IBarsCount();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSCOUNT_H_ */
