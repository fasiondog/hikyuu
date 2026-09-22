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
 * An EV whose system environment validity can only be added manually, used for the testing or other
 * purposes
 * @return EVPtr
 */
EVPtr HKU_API EV_Manual();

}  // namespace hku
