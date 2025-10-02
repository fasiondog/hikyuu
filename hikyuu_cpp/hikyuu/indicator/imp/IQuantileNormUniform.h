/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IQuantileNormUniform : public IndicatorImp {
    INDICATOR_IMP(IQuantileNormUniform)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IQuantileNormUniform();
    IQuantileNormUniform(double quantile_min, double quantile_max);
    virtual ~IQuantileNormUniform();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku