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
 * 分位数均匀分布标准化
 */
class NormQuantile : public NormalizeBase {
    NORMALIZE_IMP(NormQuantile)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormQuantile();
    NormQuantile(double quantile_min, double quantile_max);
    virtual ~NormQuantile() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku