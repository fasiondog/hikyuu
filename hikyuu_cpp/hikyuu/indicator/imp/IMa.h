/*
 * IMa.h
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR__IMP_IMA_H_
#define INDICATOR__IMP_IMA_H_

#include "../Indicator.h"

namespace hku {

class IMa : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IMa)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMa();
    virtual ~IMa();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR__IMP_IMA_H_ */
