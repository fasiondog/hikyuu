/*
 * IPow.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IPOW_H_
#define INDICATOR_IMP_IPOW_H_

#include "../Indicator.h"

namespace hku {

/**
 * 乘幂
 */
class IPow : public IndicatorImp {
    INDICATOR_IMP(IPow)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IPow();
    virtual ~IPow() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IPOW_H_ */
