/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IZScore : public IndicatorImp {
    INDICATOR_IMP(IZScore)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IZScore();
    IZScore(bool outExtreme, double nsigma, bool recursive);
    virtual ~IZScore();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku