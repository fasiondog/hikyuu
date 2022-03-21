/*
 * ISumBars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-5
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISUMBARS_H_
#define INDICATOR_IMP_ISUMBARS_H_

#include "../Indicator.h"

namespace hku {

class ISumBars : public IndicatorImp {
    INDICATOR_IMP(ISumBars)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISumBars();
    virtual ~ISumBars();

    virtual void _dyn_calculate(const Indicator&) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISUMBARS_H_ */
