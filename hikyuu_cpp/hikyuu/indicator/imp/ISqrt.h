/*
 * ISqrt.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISQRT_H_
#define INDICATOR_IMP_ISQRT_H_

#include "../Indicator.h"

namespace hku {

/**
 * 乘幂
 */
class ISqrt : public IndicatorImp {
    INDICATOR_IMP(ISqrt)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISqrt();
    virtual ~ISqrt();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISQRT_H_ */
