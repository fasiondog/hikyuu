/*
 * IAdvance.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-30
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IADVANCE_H_
#define INDICATOR_IMP_IADVANCE_H_

#include "../Indicator.h"

namespace hku {

class IAdvance : public IndicatorImp {
    INDICATOR_IMP(IAdvance)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAdvance();
    virtual ~IAdvance();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IADVANCE_H_ */
