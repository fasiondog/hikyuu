/*
 * IVar.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IVAR_H_
#define INDICATOR_IMP_IVAR_H_

#include "../Indicator.h"

namespace hku {

/*
 * 估算样本方差
 */
class IVar : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IVar)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IVar();
    virtual ~IVar();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IVAR_H_ */
