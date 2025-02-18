/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-19
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITALFUNDS_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITALFUNDS_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定资本管理策略
 * 公式: 买入数量 = 当前总资产 / capital
 * @param capital
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCapitalFunds(double capital = 10000.00);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITALFUNDS_H_ */
