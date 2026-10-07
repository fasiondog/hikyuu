/*
 * PF_WithoutAF.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 *
 *  PF is a preset configuration of MultiSystem, so this factory directly returns MultiSystemPtr,
 *  no shell class is introduced. The returned aggregate system runs in
 *  **mode C "Shared Account Compatibility"**: no shadow account is created, every sub-system
 *  trades directly on the single real account of the parent (shared_tm) and is sized by its own
 *  MM; the parent only gates the entry with the SE, keeps the running pool driven and performs no
 *  L2 conversion (see the MultiSystem class comment for the full semantics).
 *
 *  The name literally means "portfolio without a fund allocation algorithm", which is exactly what
 *  mode C is. During the three-mode refactoring this factory was temporarily mapped onto mode A;
 *  it is now restored to mode C so that it keeps reproducing the Portfolio of the previous release
 *  with no change on the strategy side, and the mode A preset moved to PF_SignalAggregate.
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_
#define TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_

#include "../../system/imp/MultiSystem.h"
#include "../../selector/SelectorBase.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"
#include "../../selector/crt/SE_Fixed.h"

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
 * @note Returns MultiSystemPtr running in mode C "Shared Account Compatibility" (the legacy
 *       Portfolio behavior): the sub-systems share the real account of the parent and are sized by
 *       their own MM (so they compete for the same cash, in the pool admission order), the AF is
 *       only the mode carrier here (L1 takes no quota, L2 needs no conversion, L3 is off by
 *       default) and the parent never places an extra order.
 * @note On a rebalancing day a sub-system that the SE stops admitting leaves the running pool no
 *       matter whether it still holds: sell_at_not_selected=true liquidates it at once, while the
 *       default false gives it ONE last drive (so a sell signal of that very day is still realized)
 *       and then stops following it (the position is left as the legacy engine did). The driven set
 *       follows the SE semantics: an "all selected" SE (e.g. SE_Fixed) degenerates mode C into
 *       driving every sub-system every day.
 * @note sys_use_self_tm is kept for the signature compatibility only: in mode C the sub-systems
 *       always share the real account of the parent, so it is ignored with a warning.
 * @param tm trade account
 * @param se system selector
 * @param adjust_cycle the rebalancing cycle (affected by adjust_mode), 1 by default
 * @param adjust_mode the rebalancing mode "query" | "day" | "week" | "month" | "year"
 * @param delay_to_trading_day when it is true, it is postponed to the first trading day within the
 *                             current cycle if the rebalancing day is not a trading day
 * @param trade_on_close execute the trade at the close
 * @param sys_use_self_tm kept for the signature compatibility only: mode C always shares the real
 *                        account of the parent with its sub-systems, so it is ignored with a
 *                        warning
 * @param sell_at_not_selected whether to force selling the stocks not selected on the rebalancing
 *                             day, false by default
 * @return the portfolio instance (MultiSystemPtr)
 * @ingroup Portfolio
 */
MultiSystemPtr HKU_API PF_WithoutAF(const TMPtr& tm = TradeManagerPtr(),
                                    const SEPtr& se = SE_Fixed(), int adjust_cycle = 1,
                                    const string& adjust_mode = "query",
                                    bool delay_to_trading_day = true, bool trade_on_close = true,
                                    bool sys_use_self_tm = false,
                                    bool sell_at_not_selected = false);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_IMP_PF_WITHOUTAF_H_ */
