/*
 * MM_FixedRisk.h
 *
 *  Created on: 2016年5月1日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定风险资金管理策略对每笔交易限定一个预先确定的或者固定的资金风险，如每笔交易固定风险1000元。
 * 公式：交易数量 = 固定风险 / 交易风险。
 * @param risk
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedRisk(double risk = 1000.00);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_ */
