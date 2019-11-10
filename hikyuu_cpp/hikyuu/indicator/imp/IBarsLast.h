/*
 * IBarsLast.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLAST_H_
#define INDICATOR_IMP_IBARSLAST_H_

#include "../Indicator.h"

namespace hku {

class IBarsLast : public IndicatorImp {
    INDICATOR_IMP(IBarsLast)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsLast();
    virtual ~IBarsLast();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSLAST_H_ */
