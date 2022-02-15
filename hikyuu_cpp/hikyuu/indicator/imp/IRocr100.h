/*
 * IRocr100.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IROCR100_H_
#define INDICATOR_IMP_IROCR100_H_

#include "../Indicator.h"

namespace hku {

class IRocr100 : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IRocr100)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRocr100();
    virtual ~IRocr100();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IROCR100_H_ */
