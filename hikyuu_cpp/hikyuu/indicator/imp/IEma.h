/*
 * IEma.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * Exponential Moving Average (EMA)
 * Parameters: n: the period window for calculating the average, it must be an integer greater than
 * 0 Discard number = 0
 */
class IEma : public IndicatorImp {
    INDICATOR_IMP(IEma)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IEma();
    virtual ~IEma() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
