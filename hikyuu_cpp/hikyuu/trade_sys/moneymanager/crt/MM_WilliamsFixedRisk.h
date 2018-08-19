/*
 * MM_WilliamsFixedRisk.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_

#include "../MoneyManagerBase.h"

namespace hku {

MoneyManagerPtr HKU_API MM_WilliamsFixedRisk(
        double p = 0.1, price_t max_loss = 1000.0);

} /* namespace */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_ */
