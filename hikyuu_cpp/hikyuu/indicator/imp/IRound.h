/*
 * IRound.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IROUND_H_
#define INDICATOR_IMP_IROUND_H_

#include "../Indicator.h"

namespace hku {

/**
 * 四舍五入
 */
class IRound : public IndicatorImp {
    INDICATOR_IMP(IRound)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRound();
    virtual ~IRound();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IROUND_H_ */
