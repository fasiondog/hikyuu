/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../NormalizeBase.h"

namespace hku {

/*
 * Quantile uniform distribution standardization
 */
class NormQuantileUniform : public NormalizeBase {
    NORMALIZE_IMP(NormQuantileUniform)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormQuantileUniform();
    NormQuantileUniform(double quantile_min, double quantile_max);
    virtual ~NormQuantileUniform() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku