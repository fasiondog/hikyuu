/*
 * MM_FixedCapital.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * @brief 连续买入/卖出固定数量资金管理策略。
 * @param buy_counts 依次买入数量
 * @param sell_counts 依次卖出数量
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_ */
