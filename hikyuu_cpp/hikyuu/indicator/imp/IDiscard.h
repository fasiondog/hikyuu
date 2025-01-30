/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDISCARD_H_
#define INDICATOR_IMP_IDISCARD_H_

#include "../Indicator.h"

namespace hku {

/* 以指标公式的方式设置抛弃值 */
class IDiscard : public IndicatorImp {
    INDICATOR_IMP(IDiscard)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDiscard();
    virtual ~IDiscard();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IDISCARD_H_ */
