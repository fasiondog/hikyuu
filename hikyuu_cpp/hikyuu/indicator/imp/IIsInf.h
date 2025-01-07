/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IISINF_H_
#define INDICATOR_IMP_IISINF_H_

#include "../Indicator.h"

namespace hku {

class IIsInf : public IndicatorImp {
    INDICATOR_IMP(IIsInf)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsInf();
    virtual ~IIsInf();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IISINF_H_ */
