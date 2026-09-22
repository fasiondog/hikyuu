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

/*
 * The position where the condition first holds within N periods; when N is 0 it is the whole
 * sequence
 * Usage: BARSSINCEN(X,N): the number of periods from the first time X is not 0 within N periods
 * until now, N is a constant BARSSINCEN(X,N):
 * For example: BARSSINCEN(HIGH>10,10) gives the number of periods from the time the stock price
 * exceeds 10 yuan within 10 periods until now
 */
class IBarsSince : public IndicatorImp {
    INDICATOR_IMP(IBarsSince)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsSince();
    virtual ~IBarsSince() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSSINCE_H_ */
