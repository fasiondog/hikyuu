/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../NormalizeBase.h"

namespace hku {

class NormMinMax : public NormalizeBase {
    NORMALIZE_IMP(NormMinMax)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormMinMax();
    virtual ~NormMinMax() override;
};

}  // namespace hku