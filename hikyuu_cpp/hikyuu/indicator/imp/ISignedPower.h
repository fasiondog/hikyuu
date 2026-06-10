/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_SIGNED_POWER_H_
#define INDICATOR_IMP_SIGNED_POWER_H_

#include "../Indicator.h"

namespace hku {

/**
 * 乘幂
 */
class ISignedPower : public IndicatorImp {
    INDICATOR_IMP(ISignedPower)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISignedPower();
    virtual ~ISignedPower() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_SIGNED_POWER_H_ */
