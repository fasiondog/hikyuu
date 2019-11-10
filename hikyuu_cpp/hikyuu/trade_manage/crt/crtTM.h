/*
 * crtTM.h
 *
 *  Created on: 2013-2-26
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTTM_H_
#define CRTTM_H_

#include "../TradeManager.h"
#include "TC_Zero.h"

namespace hku {

/**
 * 创建交易管理模块，管理帐户的交易记录及资金使用情况
 * @details 考虑的移滑价差需要使用当日的最高或最低价，所以不在该模块内进行处理
 * @ingroup TradeManagerClass
 * @param datetime 账户建立日期, 默认1990-1-1
 * @param initcash 初始现金，默认100000
 * @param costfunc 交易成本算法,默认零成本算法
 * @param name 账户名称，默认“SYS”
 * @see TradeManager
 */
TradeManagerPtr HKU_API crtTM(const Datetime& datetime = Datetime(199001010000LL),
                              price_t initcash = 100000.0, const TradeCostPtr& costfunc = TC_Zero(),
                              const string& name = "SYS");

}  // namespace hku

#endif /* CRTTM_H_ */
