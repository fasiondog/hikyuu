/*
 * IAcos.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IACOS_H_
#define INDICATOR_IMP_IACOS_H_

#include "../Indicator.h"

namespace hku {

class IAcos : public IndicatorImp {
    INDICATOR_IMP(IAcos)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAcos();
    virtual ~IAcos();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IACOS_H_ */
