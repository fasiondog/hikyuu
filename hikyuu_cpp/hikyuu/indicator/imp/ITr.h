/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-05
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITR_H_
#define INDICATOR_IMP_ITR_H_

#include "../Indicator.h"

namespace hku {

/*
 * The true range (TR) is the maximum of the following three values:
 *  1. the difference between the high price and the low price of the current period
 *  2. the absolute value of the difference between the high price of the current period and the
 *     close price of the previous period
 *  3. the absolute value of the difference between the low price of the current period and the
 *     close price of the previous period
 */
class ITr : public IndicatorImp {
    INDICATOR_IMP(ITr)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITr();
    virtual ~ITr() override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ITR_H_ */
