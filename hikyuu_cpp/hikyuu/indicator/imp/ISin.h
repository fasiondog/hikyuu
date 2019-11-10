/*
 * ISin.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISIN_H_
#define INDICATOR_IMP_ISIN_H_

#include "../Indicator.h"

namespace hku {

class ISin : public IndicatorImp {
    INDICATOR_IMP(ISin)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISin();
    virtual ~ISin();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISIN_H_ */
