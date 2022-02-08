/*
 * ConstantValue.h
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#pragma once
#ifndef INDICATOR_IMP_CONSTANTVALUE_H_
#define INDICATOR_IMP_CONSTANTVALUE_H_

#include "../Indicator.h"

namespace hku {

class ICval : public IndicatorImp {
    INDICATOR_IMP(ICval)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICval();
    ICval(double value, size_t discard);
    virtual ~ICval();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_CONSTANTVALUE_H_ */
