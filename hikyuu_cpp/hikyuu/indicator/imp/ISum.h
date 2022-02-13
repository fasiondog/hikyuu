/*
 * ISum.h
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_HKU_ISUM_H_
#define INDICATOR_IMP_HKU_ISUM_H_

#include "../Indicator.h"

namespace hku {

class ISum : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(ISum)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISum();
    virtual ~ISum();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_HKU_ISUM_H_ */
