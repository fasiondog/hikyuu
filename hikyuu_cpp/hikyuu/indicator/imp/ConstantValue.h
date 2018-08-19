/*
 * ConstantValue.h
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#ifndef INDICATOR_IMP_CONSTANTVALUE_H_
#define INDICATOR_IMP_CONSTANTVALUE_H_

#include "../Indicator.h"

namespace hku {

class ConstantValue: public IndicatorImp {
    INDICATOR_IMP(ConstantValue)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ConstantValue();
    ConstantValue(double value, size_t len, size_t discard);
    virtual ~ConstantValue();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_CONSTANTVALUE_H_ */
