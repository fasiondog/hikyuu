/*
 * ILowLineBars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILOWLINEBARS_H_
#define INDICATOR_IMP_ILOWLINEBARS_H_

#include "../Indicator.h"

namespace hku {

class ILowLineBars : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(ILowLineBars)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILowLineBars();
    virtual ~ILowLineBars();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILOWLINEBARS_H_ */
