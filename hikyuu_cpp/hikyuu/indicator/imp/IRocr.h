/*
 * IRocr.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IROCR_H_
#define INDICATOR_IMP_IROCR_H_

#include "../Indicator.h"

namespace hku {

class IRocr : public hku::IndicatorImp {
    INDICATOR_IMP(IRocr)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRocr();
    virtual ~IRocr();
    virtual void _checkParam(const string& name) const override;

    virtual bool supportIncrementCalculate() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IROCR_H_ */
