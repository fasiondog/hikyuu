/*
 * PF_Simple.cpp
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode B "Fund Allocation").
 */

#include "PF_Simple.h"

namespace hku {

MultiSystemPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                 int adjust_cycle, const string& adjust_mode,
                                 bool delay_to_trading_day, bool trade_on_close,
                                 bool sell_at_not_selected) {
    auto sys = std::make_shared<MultiSystem>("PF_Simple");
    // The preset: MultiSystem mode B "Fund Allocation" (the quota-first calibration on the
    // rebalancing day)
    sys->setAF(af ? af : AF_EqualWeight());
    sys->setMode("B");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    // Aligned with master SimplePortfolio: execute at the close, force liquidating the unselected
    // (both are configurable now, the defaults keep the legacy behavior)
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);

    HKU_WARN_IF(!tm, "PF_Simple: tm is null!");
    HKU_WARN_IF(!se, "PF_Simple: se is null!");
    return sys;
}

} /* namespace hku */
