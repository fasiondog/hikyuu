/*
 * PF_Simple.h
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_
#define TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_

#include "../Portfolio.h"
#include "../../selector/crt/SE_Fixed.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

/**
 * @brief 创建资产组合
 * @details
 * <pre>
 * 调仓模式 adjust_mode 说明：
 *  - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype
 *    决定周期间隔；
 *  - "day" 模式，adjust_cycle 为调仓间隔天数
 *  - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle
 *    为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时
 *    如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日
 *    将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月
 *    的第一个交易日
 * </pre>
 * @param tm 交易账户
 * @param se 系统选择器
 * @param af 资金分配算法
 * @param adjust_cycle 调仓周期（受 adjust_mode 影响）, 默认为1
 * @param adjust_mode 调仓模式 "query" | "day" | "week" | "month" | "year"
 * @param delay_to_trading_day true 时，如果调仓日不是交易日将会被顺延至当前周期内的第一个交易日
 * @return 组合实例
 */
PortfolioPtr HKU_API PF_Simple(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                               const AFPtr& af = AF_EqualWeight(), int adjust_cycle = 1,
                               const string& adjust_mode = "query",
                               bool delay_to_trading_day = true);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_ */
