/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_
#define TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_

#include "../Portfolio.h"
#include "../../selector/crt/SE_Fixed.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

/**
 * @brief Portfolio without a fund allocation algorithm
 * @details
 * <pre>
 * Description of the rebalancing mode adjust_mode:
 *  - In the "query" mode it follows the ktype in the input parameter query; at this time
 * adjust_cycle determines the cycle interval with the ktype in query;
 *  - In the "day" mode adjust_cycle is the number of the days between the rebalancing;
 *  - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle is the corresponding N-th day
 * of every week, the n-th day of every month, the n-th day of every quarter and the n-th day of
 * every year; when delay_to_trading_day is false, the rebalancing is skipped if that day is not a
 * trading day; when delay_to_trading_day is true, it is postponed to the first trading day within
 * the current cycle if that day is not a trading day; for example, if the rebalancing is specified
 * on the 1st day of every month but the 1st is not a trading day, it is postponed to the first
 * trading day of that month
 * </pre>
 * @note In the mode without a fund allocation algorithm, only all buying and selling at the open or
 *       all buying and selling at the close is supported!
 * @param tm trade account
 * @param se system selector
 * @param adjust_cycle the rebalancing cycle (affected by adjust_mode), 1 by default
 * @param adjust_mode the rebalancing mode "query" | "day" | "week" | "month" | "year"
 * @param delay_to_trading_day when it is true, it is postponed to the first trading day within the
 *                             current cycle if that day is not a trading day
 * @param trade_on_close execute the trade at the close
 * @param sys_use_self_tm use the trade account of the prototype system itself for the calculation
 *                        (valid in the mode without a fund allocation only), false by default
 * @param sell_at_not_selected whether to force selling the stocks not selected on the rebalancing
 *                             day, false by default
 * @return the portfolio instance
 */
PortfolioPtr HKU_API PF_WithoutAF(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                                  int adjust_cycle = 1, const string& adjust_mode = "query",
                                  bool delay_to_trading_day = true, bool trade_on_close = true,
                                  bool sys_use_self_tm = false, bool sell_at_not_selected = false);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_ */
