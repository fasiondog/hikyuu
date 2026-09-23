/*
 * PF_WithoutAF.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 *
 *  v5: the PF compatibility layer factory implementation -- the factory passes through to MultiSystem (mode A).
 *  See docs/design/pf_af_compat/design.md §4
 */

#include "PF_WithoutAF.h"

namespace hku {

MultiSystemPtr HKU_API PF_WithoutAF(const TMPtr& tm, const SEPtr& se, int adjust_cycle,
                                    const string& adjust_mode, bool delay_to_trading_day,
                                    bool trade_on_close, bool sys_use_self_tm,
                                    bool sell_at_not_selected) {
    auto sys = std::make_shared<MultiSystem>("PF_WithoutAF");
    // The essence of PF_WithoutAF: MultiSystem mode A (signal aggregation + the parent uniform ordering, see design.md §4.2 / §4.4)
    sys->setAF(AF_EqualWeight());
    sys->setMode("A");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);

    // sys_use_self_tm has no corresponding semantics in the new system (the sub-systems always use the shadow account TM_SUB, shared_tm=false)
    HKU_WARN_IF(sys_use_self_tm,
                "PF_WithoutAF: sys_use_self_tm is ignored (no equivalent in MultiSystem)!");
    HKU_WARN_IF(!tm, "PF_WithoutAF: tm is null!");
    HKU_WARN_IF(!se, "PF_WithoutAF: se is null!");
    return sys;
}

} /* namespace hku */
