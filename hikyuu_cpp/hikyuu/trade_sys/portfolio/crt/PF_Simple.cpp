/*
 * PF_Simple.cpp
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 *
 *  v5: the PF compatibility layer factory implementation -- the factory passes through to MultiSystem (mode B).
 *  See docs/design/pf_af_compat/design.md §4
 */

#include "PF_Simple.h"

namespace hku {

MultiSystemPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                 int adjust_cycle, const string& adjust_mode,
                                 bool delay_to_trading_day) {
    auto sys = std::make_shared<MultiSystem>("PF_Simple");
    // The essence of PF: MultiSystem mode B (quota allocation + the next-period quota write-back, see design.md §4.2 / §4.4)
    // Note: call setAF first then setMode, so that setMode takes effect on the current AF.
    sys->setAF(af ? af : AF_EqualWeight());
    sys->setMode("B");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    // Aligned with master SimplePortfolio: execute at the close, force liquidating the unselected
    sys->setTradeOnClose(true);
    sys->setSellAtNotSelected(true);

    HKU_WARN_IF(!tm, "PF_Simple: tm is null!");
    HKU_WARN_IF(!se, "PF_Simple: se is null!");
    return sys;
}

} /* namespace hku */
