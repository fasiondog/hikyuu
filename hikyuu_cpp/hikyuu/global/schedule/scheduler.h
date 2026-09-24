/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "../../utilities/TimerManager.h"

namespace hku {

/**
 * Get the pointer of the global scheduler instance
 */
HKU_API TimerManager* getScheduler();

/**
 * Used to release the global scheduler instance on program exit, for internal use only
 */
void releaseScheduler();

}  // namespace hku