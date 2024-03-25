/*
 * IAtr.h
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#pragma once
#ifndef INDICATOR_IMP_ATR_H_
#define INDICATOR_IMP_ATR_H_

#include "../Indicator.h"

namespace hku {

class IAtr : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IAtr)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAtr();
    virtual ~IAtr();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ATR_H_ */
