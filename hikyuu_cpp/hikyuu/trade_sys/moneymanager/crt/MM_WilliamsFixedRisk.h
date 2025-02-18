/*
 * MM_WilliamsFixedRisk.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * @brief 威廉斯固定风险资金管理策略
 * 买入数量 =（账户余额 × 风险百分比p）÷ 最大损失(max_loss)
 * @param p 风险百分比
 * @param max_loss 最大损失
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_WilliamsFixedRisk(double p = 0.1, price_t max_loss = 1000.0);

}  // namespace hku

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_WILLIAMSFIXEDRISK_H_ */
