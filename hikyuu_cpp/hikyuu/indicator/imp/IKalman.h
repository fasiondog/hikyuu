/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IKALMAN_H_
#define INDICATOR_IMP_IKALMAN_H_

#include "../Indicator.h"

namespace hku {

/* kalman滤波 */
class IKalman : public IndicatorImp {
    INDICATOR_IMP(IKalman)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IKalman();
    virtual ~IKalman();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IKALMAN_H_ */
