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
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILowLine();
    virtual ~ILowLine();
    virtual void _dyn_calculate(const Indicator& data) override;

private:
    static std::vector<price_t> _get_one_step(const Indicator& data, size_t pos, size_t num,
                                              size_t step);
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ILOWLINE_H_ */
