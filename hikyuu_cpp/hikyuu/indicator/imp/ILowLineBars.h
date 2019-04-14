/*
 * ILowLineBars.h
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_ILOWLINEBARS_H_
#define INDICATOR_IMP_ILOWLINEBARS_H_

#include "../Indicator.h"

namespace hku {

class ILowLineBars: public IndicatorImp {
    INDICATOR_IMP(ILowLineBars)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILowLineBars();
    virtual ~ILowLineBars();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILOWLINEBARS_H_ */
