/*
 * SL_FixedPercent.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_
#define TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_

#include "../SlippageBase.h"

namespace hku {

/**
 * 固定百分比移滑价差算法
 * @details 买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)
 * @param p 偏移的固定百分比
 * @return SPPtr
 */
SlippagePtr HKU_API SP_FixedPercent(double p = 0.001);

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_ */
