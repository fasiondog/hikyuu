/*
 * IAlign.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IALIGH_H_
#define INDICATOR_IMP_IALIGH_H_

#include "../Indicator.h"

namespace hku {

class IAlign : public IndicatorImp {
    INDICATOR_IMP(IAlign)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAlign();
    virtual ~IAlign();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IALIGH_H_ */
