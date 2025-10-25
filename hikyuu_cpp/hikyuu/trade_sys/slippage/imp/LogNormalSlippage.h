/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

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
    static std::random_device ms_rd;
    static std::mt19937 ms_gen;
};

} /* namespace hku */