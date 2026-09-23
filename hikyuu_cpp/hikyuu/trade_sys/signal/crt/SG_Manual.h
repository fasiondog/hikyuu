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
 * A signal generator whose signals can only be added manually, used for the testing or other
 * special purposes
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Manual();

}  // namespace hku