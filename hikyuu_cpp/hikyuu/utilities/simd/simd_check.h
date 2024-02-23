/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20230222 added by fasiondog
 */

#pragma once

#include "../osdef.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

void HKU_API checkUnavailableButEnabledCpuInstructions();

}  // namespace hku