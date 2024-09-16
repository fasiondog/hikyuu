/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../ConditionBase.h"

namespace hku {

/**
 * 仅能手工添加有效性的CN，用于测试或其他特殊用途
 * @return CNPtr
 */
CNPtr HKU_API CN_Manual();

}  // namespace hku