/*
 * IHighLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016-4-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHIGHLINE_H_
#define INDICATOR_IMP_IHIGHLINE_H_

#include "../Indicator.h"

namespace hku {

/*
 * The highest price within N days, the high price data is generally used as the input
 * Parameters: n: N-day time window
 */
class IHighLine : public IndicatorImp {
    INDICATOR_IMP(IHighLine)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHighLine();
    virtual ~IHighLine() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_IHIGHLINE_H_ */
