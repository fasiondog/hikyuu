/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../EnvironmentBase.h"

namespace hku {

/**
 * 仅能手工添加系统环境有效性的EV，用于测试或其他用途
 * @return EVPtr
 */
EVPtr HKU_API EV_Manual();

}  // namespace hku
