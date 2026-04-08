/*
 * IBarsLasts.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLASTS_H_
#define INDICATOR_IMP_IBARSLASTS_H_

#include "../Indicator.h"

namespace hku {

class IBarsLasts : public IndicatorImp {
    INDICATOR_IMP(IBarsLasts)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsLasts();
    virtual ~IBarsLasts();

    virtual void _dyn_calculate(const Indicator&) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSLASTS_H_ */
