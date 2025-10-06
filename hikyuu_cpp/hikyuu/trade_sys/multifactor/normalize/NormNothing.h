/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../NormalizeBase.h"

namespace hku {

class NormNothing : public NormalizeBase {
    NORMALIZE_IMP(NormNothing)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormNothing();
    virtual ~NormNothing();
};

}  // namespace hku