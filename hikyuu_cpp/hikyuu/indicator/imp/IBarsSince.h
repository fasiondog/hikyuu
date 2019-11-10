/*
 * IBarsSince.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSSINCE_H_
#define INDICATOR_IMP_IBARSSINCE_H_

#include "../Indicator.h"

namespace hku {

class IBarsSince : public IndicatorImp {
    INDICATOR_IMP(IBarsSince)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsSince();
    virtual ~IBarsSince();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSSINCE_H_ */
