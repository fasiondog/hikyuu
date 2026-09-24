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
 * A CN whose validity can only be added manually, used for the testing or other special purposes
 * @return CNPtr
 */
CNPtr HKU_API CN_Manual();

}  // namespace hku