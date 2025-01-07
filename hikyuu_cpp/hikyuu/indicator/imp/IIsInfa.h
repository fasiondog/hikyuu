/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IISINFA_H_
#define INDICATOR_IMP_IISINFA_H_

#include "../Indicator.h"

namespace hku {

class IIsInfa : public IndicatorImp {
    INDICATOR_IMP(IIsInfa)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsInfa();
    virtual ~IIsInfa();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IISINFA_H_ */
