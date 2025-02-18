/*
 * MM_FixedCapital.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITAL_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITAL_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定资金管理策略
 * 公式: 买入数量 = 当前现金 / capital
 * @param capital
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCapital(double capital = 10000.00);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITAL_H_ */
