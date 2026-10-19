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
 *
 * @note Not exported to Python: ManualCondition keeps no own cache, so clone/combination setTO
 *       wipes the manually added values (see the condition audit plan).
 * @return CNPtr
 */
CNPtr HKU_API CN_Manual();

}  // namespace hku