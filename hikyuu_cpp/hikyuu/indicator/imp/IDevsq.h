/*
 * IDevsq.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDEVSQ_H_
#define INDICATOR_IMP_IDEVSQ_H_

#include "../Indicator.h"

namespace hku {

class IDevsq : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IDevsq)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDevsq();
    virtual ~IDevsq();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IDEVSQ_H_ */
