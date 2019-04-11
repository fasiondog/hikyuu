/*
 * IHhvbars.h
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_IHHVBARS_H_
#define INDICATOR_IMP_IHHVBARS_H_

#include "../Indicator.h"

namespace hku {

class IHhvbars: public IndicatorImp {
    INDICATOR_IMP(IHhvbars)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHhvbars();
    virtual ~IHhvbars();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IHHVBARS_H_ */
