/*
 * IIntpart.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IINTPART_H_
#define INDICATOR_IMP_IINTPART_H_

#include "../Indicator.h"

namespace hku {

/*
 * Round to an integer (rounded toward a smaller absolute value, i.e. the integer part of the data)
 */
class IIntpart : public IndicatorImp {
    INDICATOR_IMP(IIntpart)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIntpart();
    virtual ~IIntpart() override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IINTPART_H_ */
