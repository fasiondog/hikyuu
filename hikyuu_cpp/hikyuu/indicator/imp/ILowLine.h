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
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(ILowLine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILowLine();
    virtual ~ILowLine();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ILOWLINE_H_ */
