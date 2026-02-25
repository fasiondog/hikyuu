/*
 * ILowLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILOWLINE_H_
#define INDICATOR_IMP_ILOWLINE_H_

#include "../Indicator.h"

namespace hku {

class ILowLine : public IndicatorImp {
    INDICATOR_IMP(ILowLine)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILowLine();
    virtual ~ILowLine() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ILOWLINE_H_ */
