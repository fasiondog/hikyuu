/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../ConditionBase.h"

namespace hku {

class ManualCondition : public ConditionBase {
    CONDITION_IMP(ManualCondition)
    CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ManualCondition();
    virtual ~ManualCondition() = default;
};

}  // namespace hku