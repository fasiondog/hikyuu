/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

/**
 * 仅能手动添加信号的信号指示器，用于测试或其他特殊用途
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Manual();

}  // namespace hku