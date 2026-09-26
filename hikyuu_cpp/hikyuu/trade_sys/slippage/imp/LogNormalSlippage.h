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

class LogNormalSlippage : public SlippageBase {
    SLIPPAGE_IMP(LogNormalSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    LogNormalSlippage();
    virtual ~LogNormalSlippage();
    virtual void _checkParam(const string& name) const override;

private:
    std::mt19937 m_gen;
};

} /* namespace hku */