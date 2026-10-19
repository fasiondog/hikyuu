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

class UniformSlippage : public SlippageBase {
    SLIPPAGE_IMP_RNG(UniformSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    UniformSlippage();
    virtual ~UniformSlippage();
    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual price_t _getRealBuyShortPrice(const Datetime&, price_t) override;
    virtual price_t _getRealSellShortPrice(const Datetime&, price_t) override;

private:
    void _checkRange(const string& min_name, const string& max_name) const;
    std::mt19937 m_gen;
};

} /* namespace hku */
