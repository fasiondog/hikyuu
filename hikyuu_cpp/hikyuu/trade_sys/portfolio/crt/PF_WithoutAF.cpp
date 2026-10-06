/*
 * PF_WithoutAF.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode C "Shared Account Compatibility", i.e. the legacy Portfolio without a fund allocation
 *  algorithm).
 */

#include "PF_WithoutAF.h"

namespace hku {

MultiSystemPtr HKU_API PF_WithoutAF(const TMPtr& tm, const SEPtr& se, int adjust_cycle,
                                    const string& adjust_mode, bool delay_to_trading_day,
                                    bool trade_on_close, bool sys_use_self_tm,
                                    bool sell_at_not_selected) {
    auto sys = std::make_shared<MultiSystem>("PF_WithoutAF");
    // The preset: MultiSystem mode C "Shared Account Compatibility" (the legacy Portfolio: the
    // sub-systems trade on the shared real account and are sized by their own MM). The AF is only
    // the mode carrier here: L1 takes no quota, L2 needs no conversion, L3 is off by default.
    sys->setAF(AF_EqualWeight());
    sys->setMode("C");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);

    // Mode C always shares the real account of the parent, so the "own tm of the prototype
    // system" branch of the legacy engine is not exposed as a per-layer capability (C-O3=A)
    HKU_WARN_IF(sys_use_self_tm,
                "PF_WithoutAF: sys_use_self_tm is ignored (mode C always shares the real account "
                "of the parent)!");
    HKU_WARN_IF(!tm, "PF_WithoutAF: tm is null!");
    HKU_WARN_IF(!se, "PF_WithoutAF: se is null!");
    return sys;
}

} /* namespace hku */
