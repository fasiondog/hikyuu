/*
 * PF_WithoutAF.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 *
 *  v5：PF 兼容层工厂实现 —— 工厂直通到 MultiSystem（模式 A）。
 *  见 docs/design/pf_af_compat/design.md §4
 */

#include "PF_WithoutAF.h"

namespace hku {

MultiSystemPtr HKU_API PF_WithoutAF(const TMPtr& tm, const SEPtr& se, int adjust_cycle,
                                    const string& adjust_mode, bool delay_to_trading_day,
                                    bool trade_on_close, bool sys_use_self_tm,
                                    bool sell_at_not_selected) {
    auto sys = std::make_shared<MultiSystem>("PF_WithoutAF");
    // PF_WithoutAF 本质：MultiSystem 模式 A（信号汇总 + 父统一下单，见 design.md §4.2 / §4.4）
    sys->setAF(AF_EqualWeight());
    sys->setMode("A");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);

    // sys_use_self_tm 在新体系无对应语义（子系统恒用影子账户 TM_SUB，shared_tm=false）
    HKU_WARN_IF(sys_use_self_tm,
                "PF_WithoutAF: sys_use_self_tm is ignored (no equivalent in MultiSystem)!");
    HKU_WARN_IF(!tm, "PF_WithoutAF: tm is null!");
    HKU_WARN_IF(!se, "PF_WithoutAF: se is null!");
    return sys;
}

} /* namespace hku */
