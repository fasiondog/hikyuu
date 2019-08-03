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

MoneyManagerPtr HKU_API MM_FixedRisk(double risk = 1000.00);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDRISK_H_ */
