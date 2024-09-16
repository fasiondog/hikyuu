/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

class ManualSignal : public SignalBase {
    SIGNAL_IMP(ManualSignal)
    SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ManualSignal();
    virtual ~ManualSignal() = default;
};

}  // namespace hku