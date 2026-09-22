/*
 * ICeil.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICEIL_H_
#define INDICATOR_IMP_ICEIL_H_

#include "../Indicator.h"

namespace hku {

/**
 * Round up (round in the direction of increasing value) to an integer
 */
class ICeil : public IndicatorImp {
    INDICATOR_IMP(ICeil)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICeil();
    virtual ~ICeil() override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ICEIL_H_ */
