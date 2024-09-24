/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../EnvironmentBase.h"

namespace hku {

class ManualEnvironment : public EnvironmentBase {
    ENVIRONMENT_IMP(ManualEnvironment)
    ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ManualEnvironment();
    virtual ~ManualEnvironment() = default;
};

}  // namespace hku