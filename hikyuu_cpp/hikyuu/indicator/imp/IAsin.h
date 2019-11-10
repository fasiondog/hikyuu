/*
 * IAsin.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IASIN_H_
#define INDICATOR_IMP_IASIN_H_

#include "../Indicator.h"

namespace hku {

class IAsin : public IndicatorImp {
    INDICATOR_IMP(IAsin)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAsin();
    virtual ~IAsin();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IASIN_H_ */
