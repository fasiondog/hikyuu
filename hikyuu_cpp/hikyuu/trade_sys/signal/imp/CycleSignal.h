/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-01
 *      Author: fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

class CycleSignal : public SignalBase {
    SIGNAL_IMP(CycleSignal)
    SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    CycleSignal();
    virtual ~CycleSignal() = default;

    virtual void _checkParam(const string& name) const override;
};

}