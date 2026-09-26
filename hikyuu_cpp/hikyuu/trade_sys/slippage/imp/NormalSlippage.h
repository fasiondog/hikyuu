/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include <random>
#include "../SlippageBase.h"

namespace hku {

class NormalSlippage : public SlippageBase {
    SLIPPAGE_IMP(NormalSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormalSlippage();
    virtual ~NormalSlippage();
    virtual void _checkParam(const string& name) const override;

private:
    std::mt19937 m_gen;
};

} /* namespace hku */