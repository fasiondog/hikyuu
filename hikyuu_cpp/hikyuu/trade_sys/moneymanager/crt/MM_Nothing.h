/*
 * MM_FixedCount.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef NOTHING_MM_H_
#define NOTHING_MM_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * No money management is done, it buys as much as the available money allows
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_Nothing();

}  // namespace hku

#endif /* NOTHING_MM_H_ */
