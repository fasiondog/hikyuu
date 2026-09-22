/*
 * MM_FixedRisk.h
 *
 *  Created on: 2016-5-1
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * The fixed risk money management strategy limits a predetermined or fixed fund risk for every
 * trade, such as a fixed risk of 1000 yuan for every trade.
 * Formula: trade quantity = fixed risk / trade risk.
 * @param risk
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedRisk(double risk = 1000.00);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_ */
