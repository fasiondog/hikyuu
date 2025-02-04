/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILASTVALUE_H_
#define INDICATOR_IMP_ILASTVALUE_H_

#include "../Indicator.h"

namespace hku {

class ILastValue : public IndicatorImp {
    INDICATOR_IMP(ILastValue)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILastValue();
    virtual ~ILastValue();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILASTVALUE_H_ */
