/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLASTCOUNT_H_
#define INDICATOR_IMP_IBARSLASTCOUNT_H_

#include "../Indicator.h"

namespace hku {

/* Count the number of the consecutive periods satisfying the condition */
class IBarsLastCount : public IndicatorImp {
    INDICATOR_IMP(IBarsLastCount)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsLastCount();
    virtual ~IBarsLastCount() override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSLASTCOUNT_H_ */
