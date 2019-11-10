/*
 * IAtan.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IATAN_H_
#define INDICATOR_IMP_IATAN_H_

#include "../Indicator.h"

namespace hku {

class IAtan : public IndicatorImp {
    INDICATOR_IMP(IAtan)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAtan();
    virtual ~IAtan();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IATAN_H_ */
