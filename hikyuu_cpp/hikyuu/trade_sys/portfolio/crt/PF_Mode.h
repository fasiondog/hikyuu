/*
 * PF_Mode.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-17
 *      Author: fasiondog
 *
 *  The alphabetical alias factories of the three running modes (inline forwarders of the
 *  semantic-name factories, no behavioral difference).
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_
#define TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_

#include "PF_SignalAggregate.h"
#include "PF_Simple.h"
#include "PF_WithoutAF.h"

namespace hku {

/** Alias of PF_SignalAggregate: mode A "Signal Aggregation" @ingroup Portfolio */
inline MultiSystemPtr PF_ModeA(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                               const AFPtr& af = AF_EqualWeight(), int adjust_cycle = 1,
                               const string& adjust_mode = "query",
                               bool delay_to_trading_day = true, bool trade_on_close = true,
                               bool sell_at_not_selected = false,
                               price_t sub_init_cash = 100000.0) {
    return PF_SignalAggregate(tm, se, af, adjust_cycle, adjust_mode, delay_to_trading_day,
                              trade_on_close, sell_at_not_selected, sub_init_cash);
}

/** Alias of PF_Simple: mode B "Fund Allocation" @ingroup Portfolio */
inline MultiSystemPtr PF_ModeB(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                               const AFPtr& af = AF_EqualWeight(), int adjust_cycle = 1,
                               const string& adjust_mode = "query",
                               bool delay_to_trading_day = true, bool trade_on_close = true,
                               bool sell_at_not_selected = true) {
    return PF_Simple(tm, se, af, adjust_cycle, adjust_mode, delay_to_trading_day, trade_on_close,
                     sell_at_not_selected);
}

/** Alias of PF_WithoutAF: mode C "Shared Account Compatibility" @ingroup Portfolio */
inline MultiSystemPtr PF_ModeC(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                               int adjust_cycle = 1, const string& adjust_mode = "query",
                               bool delay_to_trading_day = true, bool trade_on_close = true,
                               bool sys_use_self_tm = false, bool sell_at_not_selected = false) {
    return PF_WithoutAF(tm, se, adjust_cycle, adjust_mode, delay_to_trading_day, trade_on_close,
                        sys_use_self_tm, sell_at_not_selected);
}

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_ */
