/*
 * MM_FixedCount.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDCOUNT_MM_H_
#define FIXEDCOUNT_MM_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定交易数量资金管理策略
 * @details 每次买入固定的数量。
 * @param n 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
 * @note 1) 该策略主要用于测试和其他策略进行比较结果，本身不符合现实。\n
 *       2) 该策略并不判断已有的持仓情况，如果在已有持仓情况下不能交易，则该判断应为System本身的责任
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_FixedCount(double n = 100);

}  // namespace hku

#endif /* FIXEDCOUNT_MM_H_ */
