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
 * 不做资金管理，有多少钱买多少
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_Nothing();

}  // namespace hku

#endif /* NOTHING_MM_H_ */
