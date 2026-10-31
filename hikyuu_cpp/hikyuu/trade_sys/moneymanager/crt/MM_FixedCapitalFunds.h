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
 * Fixed capital funds management strategy
 * Formula: buy quantity = current net assets / capital
 * @param capital
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCapitalFunds(double capital = 10000.00);

void registerPart_MM_FixedCapitalFunds();

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCAPITALFUNDS_H_ */
