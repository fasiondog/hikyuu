/*
 * ITan.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITAN_H_
#define INDICATOR_IMP_ITAN_H_

#include "../Indicator.h"

namespace hku {

class ITan : public IndicatorImp {
    INDICATOR_IMP(ITan)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITan();
    virtual ~ITan();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ITAN_H_ */
