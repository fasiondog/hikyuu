/*
 * IEvery.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IEVERY_H_
#define INDICATOR_IMP_IEVERY_H_

#include "../Indicator.h"

namespace hku {

/*
 * 一直存在, EVERY (X,N) 表示条件X在N周期一直存在
 */
class IEvery : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IEvery)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IEvery();
    virtual ~IEvery();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IEVERY_H_ */
