/*
 * PF_SignalAggregate.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-06
 *      Author: fasiondog
 *
 *  PF is a preset configuration of MultiSystem, so this factory directly returns MultiSystemPtr,
 *  no shell class is introduced. The returned aggregate system runs in
 *  **mode A "Signal Aggregation"**: the sub-systems are pure signal sources on their shadow
 *  accounts (the signal cash is reset on every rebalancing day), the parent converts the
 *  suggestions into the executable quantity by the L2 target conversion and orders uniformly on
 *  the parent account (see the MultiSystem class comment for the full semantics).
 *
 *  Before the three-mode refactoring this preset was carried by PF_WithoutAF, whose name now
 *  stands for mode C (the legacy shared-account Portfolio) again.
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_CRT_PF_SIGNALAGGREGATE_H_
#define TRADE_SYS_PORTFOLIO_CRT_PF_SIGNALAGGREGATE_H_

#include "../../system/imp/MultiSystem.h"
#include "../../selector/SelectorBase.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"
#include "../../selector/crt/SE_Fixed.h"

namespace hku {

/**
 * @brief Portfolio running in mode A "Signal Aggregation" (the signal sources + the parent uniform
 *        ordering)
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
 * the current cycle if that day is not a trading day
 * </pre>
 * @note The sub-systems always use their own shadow accounts (TM_SUB, shared_tm=false), so the real
 *       account of the parent is only touched by the orders of the parent itself. The suggestions
 *       are converted into the target positions by the AF L2 (weight x position ratio x the parent
 *       total assets, aggregated per instrument) and the L3 portfolio risk control applies. On a
 *       rebalancing day the sub-systems that the SE does not admit are skipped.
 * @param tm trade account
 * @param se system selector
 * @param af the portfolio-level fund allocation algorithm (AF, carrying L1/L2/L3), equal weight by
 *           default
 * @param adjust_cycle the rebalancing cycle (affected by adjust_mode), 1 by default
 * @param adjust_mode the rebalancing mode "query" | "day" | "week" | "month" | "quarter" |
 *                    "year"
 * @param delay_to_trading_day when it is true, it is postponed to the first trading day within the
 *                             current cycle if the rebalancing day is not a trading day
 * @param trade_on_close execute the trade at the close
 * @param sell_at_not_selected whether to force selling the stocks not selected on the rebalancing
 *                             day, false by default
 * @param sub_init_cash the signal cash of every sub-system shadow account, reset on every
 *                      rebalancing day
 * @return the portfolio instance (MultiSystemPtr)
 * @ingroup Portfolio
 */
MultiSystemPtr HKU_API PF_SignalAggregate(
  const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
  const AFPtr& af = AF_EqualWeight(), int adjust_cycle = 1, const string& adjust_mode = "query",
  bool delay_to_trading_day = true, bool trade_on_close = true, bool sell_at_not_selected = false,
  price_t sub_init_cash = 100000.0);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_CRT_PF_SIGNALAGGREGATE_H_ */
