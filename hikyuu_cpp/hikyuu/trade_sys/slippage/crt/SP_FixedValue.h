/*
 * SL_FixedValue.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_
#define TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_

#include "../SlippageBase.h"

namespace hku {

/**
 * 固定价格移滑价差算法
 * @details 买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格
 * @param value
 * @return
 */
SlippagePtr HKU_API SP_FixedValue(double value = 0.01);

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_ */
