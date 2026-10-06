/*
 * PF_SignalAggregate.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-06
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode A "Signal Aggregation").
 */

#include "PF_SignalAggregate.h"

namespace hku {

MultiSystemPtr HKU_API PF_SignalAggregate(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                          int adjust_cycle, const string& adjust_mode,
                                          bool delay_to_trading_day, bool trade_on_close,
                                          bool sell_at_not_selected, price_t sub_init_cash) {
    auto sys = std::make_shared<MultiSystem>("PF_SignalAggregate");
    // The preset: MultiSystem mode A "Signal Aggregation" (the signal sources + the parent uniform
    // ordering). The shadow account of every sub-system is funded with sub_init_cash and reset on
    // every rebalancing day, so a sub-system keeps submitting a fresh position intent.
    sys->setAF(af == nullptr ? AF_EqualWeight() : af);
    sys->setMode("A");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);
    sys->setSubInitCash(sub_init_cash);

    HKU_WARN_IF(!tm, "PF_SignalAggregate: tm is null!");
    HKU_WARN_IF(!se, "PF_SignalAggregate: se is null!");
    return sys;
}

} /* namespace hku */
