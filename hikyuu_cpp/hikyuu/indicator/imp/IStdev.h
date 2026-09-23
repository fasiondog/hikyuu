/*
 * IStdev.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * Calculate the sample standard deviation within N periods
 * Parameters: n: N-day time window
 * TODO      ma : the function prototype for calculating the average
 *       link: the linkage flag of the average parameters, true by default
 *
 */
class IStdev : public hku::IndicatorImp {
    INDICATOR_IMP(IStdev)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IStdev();
    virtual ~IStdev() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
