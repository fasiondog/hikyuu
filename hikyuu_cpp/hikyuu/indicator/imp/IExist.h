/*
 * IExist.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IEXIST_H_
#define INDICATOR_IMP_IEXIST_H_

#include "../Indicator.h"

namespace hku {

/*
 * 存在, EXIST(X,N) 表示条件X在N周期有存在
 */
class IExist : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IExist)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IExist();
    virtual ~IExist();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IEXIST_H_ */
