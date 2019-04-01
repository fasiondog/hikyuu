/*
 * ISum.h
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#ifndef HKU_ISUM_H_
#define HKU_ISUM_H_

#include "../Indicator.h"

namespace hku {

class ISum: public IndicatorImp {
    INDICATOR_IMP(ISum)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISum();
    virtual ~ISum();
};

} /* namespace hku */
#endif /* HKU_ISUM_H_ */
