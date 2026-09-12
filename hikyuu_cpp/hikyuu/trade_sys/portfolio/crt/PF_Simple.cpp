/*
 * PF_Simple.cpp
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 *
 *  v5：PF 兼容层工厂实现 —— 工厂直通到 MultiSystem（模式 B）。
 *  见 docs/design/pf_af_compat/design.md §4
 */

#include "PF_Simple.h"

namespace hku {

MultiSystemPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                 int adjust_cycle, const string& adjust_mode,
                                 bool delay_to_trading_day) {
    auto sys = std::make_shared<MultiSystem>("PF_Simple");
    // PF 本质：MultiSystem 模式 B（额度划拨 + 下期额度回写，见 design.md §4.2 / §4.4）
    // 注意：先 setAF 再 setMode，setMode 才会作用到当前 AF。
    sys->setAF(af ? af : AF_EqualWeight());
    sys->setMode("B");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    // 与 master SimplePortfolio 对齐：收盘执行、未选中强制清仓
    sys->setTradeOnClose(true);
    sys->setSellAtNotSelected(true);

    HKU_WARN_IF(!tm, "PF_Simple: tm is null!");
    HKU_WARN_IF(!se, "PF_Simple: se is null!");
    return sys;
}

} /* namespace hku */
