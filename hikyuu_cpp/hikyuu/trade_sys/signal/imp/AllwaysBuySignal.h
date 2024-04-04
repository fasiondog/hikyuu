/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

class AllwaysBuySignal : public SignalBase {
    SIGNAL_IMP(AllwaysBuySignal)
    SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    AllwaysBuySignal();
    virtual ~AllwaysBuySignal() = default;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku