/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IISNA_H_
#define INDICATOR_IMP_IISNA_H_

#include "../Indicator.h"

namespace hku {

class IIsNa : public IndicatorImp {
    INDICATOR_IMP(IIsNa)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsNa();
    virtual ~IIsNa();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IISNA_H_ */
