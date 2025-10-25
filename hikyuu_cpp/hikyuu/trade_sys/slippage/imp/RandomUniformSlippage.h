/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

class RandomUniformSlippage : public SlippageBase {
    SLIPPAGE_IMP(RandomUniformSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    RandomUniformSlippage();
    virtual ~RandomUniformSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
