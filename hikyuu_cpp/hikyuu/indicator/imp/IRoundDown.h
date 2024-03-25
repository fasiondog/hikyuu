/*
 * IRoundDown.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IROUNDDOWN_H_
#define INDICATOR_IMP_IROUNDDOWN_H_

#include "../Indicator.h"

namespace hku {

/**
 * 向下截取，如10.1截取后为10
 */
class IRoundDown : public IndicatorImp {
    INDICATOR_IMP(IRoundDown)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRoundDown();
    virtual ~IRoundDown();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IROUNDDOWN_H_ */
