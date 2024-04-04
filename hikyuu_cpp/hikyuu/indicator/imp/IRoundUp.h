/*
 * IRoundUp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IROUNDUP_H_
#define INDICATOR_IMP_IROUNDUP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 向上截取，如10.1截取后为11
 */
class IRoundUp : public IndicatorImp {
    INDICATOR_IMP(IRoundUp)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRoundUp();
    virtual ~IRoundUp();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IROUNDUP_H_ */
