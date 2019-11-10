/*
 * IBackset.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBACKSET_H_
#define INDICATOR_IMP_IBACKSET_H_

#include "../Indicator.h"

namespace hku {

/*
 * 存在, EXIST(X,N) 表示条件X在N周期有存在
 */
class IBackset : public IndicatorImp {
    INDICATOR_IMP(IBackset)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBackset();
    virtual ~IBackset();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBACKSET_H_ */
