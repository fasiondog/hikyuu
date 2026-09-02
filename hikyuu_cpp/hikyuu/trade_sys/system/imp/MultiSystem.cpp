/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  递归组合重构：聚合交易系统（组合回测）
 *  阶段 3：双模式（A/B）+ 任意嵌套 + MM L1/L2/L3 + 调仓周期 + 层级路径。
 */

#include "MultiSystem.h"

#include <map>
#include <cmath>
#include <set>

#include "../../selector/SelectorBase.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiSystem)
#endif

namespace hku {

bool MultiSystem::_subtreeContains(const SystemPtr& candidate, System* target) {
    if (!candidate || !target) {
        return false;
    }
    if (candidate.get() == target) {
        return true;
    }
    if (auto* ms = dynamic_cast<MultiSystem*>(candidate.get())) {
        for (auto& s : ms->m_sys_list) {
            if (_subtreeContains(s, target)) {
                return true;
            }
        }
    }
    return false;
}

void MultiSystem::add(const SystemPtr& sys) {
    HKU_WARN_IF_RETURN(!sys, void(), "Null subsystem!");
    HKU_WARN_IF_RETURN(_subtreeContains(sys, this), void(), "Cyclic reference detected! {}", name());
    for (auto& s : m_sys_list) {
        if (s.get() == sys.get()) {
            HKU_WARN("Subsystem already exists, ignored! {}", name());
            return;
        }
    }
    m_sys_list.push_back(sys);
}

void MultiSystem::readyForRun() {
    HKU_CHECK(m_tm, "Not setTradeManager! {}", name());
    HKU_CHECK(!m_sys_list.empty(), "No subsystem specified! {}", name());

    // 聚合形态不校验自身 SG/MM/ST 等单证券组件（它们属于各子系统）；
    // 仅准备好运行环境，子系统的组件在校验循环中各子 readyForRun 完成。
    m_close_day_index = 0;
    if (m_path.empty()) {
        m_path = name();
    }

    // 为每个子系统创建独立虚拟账户（模式 A：固定影子账户；模式 B：由父分配额度）
    // 子系统保留各自 SG/MM/EV/CN/ST/TP/PG/SP（各自独立证券的策略），仅账户隔离。
    for (auto& sys : m_sys_list) {
        TMPtr sub_tm = crtTM(m_tm->initDatetime(), m_sub_init_cash, TC_Zero(), "TM_SUB");
        sys->setTM(sub_tm);
        sys->setParam<bool>("shared_tm", false);
        // 层级路径递归写入
        sys->setPath(m_path + "/" + sys->name());
        if (!sys->getTO().empty()) {
            sys->setTO(sys->getTO());
        }
        sys->readyForRun();
    }
}

void MultiSystem::_reset() {
    for (auto& sys : m_sys_list) {
        sys->reset();
    }
    m_trade_list.clear();
    m_buyRequestList.clear();
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
    m_buyShortRequestList.clear();
    m_last_suggestions.clear();
    m_open_trades.clear();
    m_adjust_turnover.clear();
}

void MultiSystem::_forceResetAll() {
    for (auto& sys : m_sys_list) {
        sys->forceResetAll();
    }
    m_trade_list.clear();
    m_buyRequestList.clear();
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
    m_buyShortRequestList.clear();
    m_last_suggestions.clear();
    m_open_trades.clear();
    m_adjust_turnover.clear();
}

SystemPtr MultiSystem::_clone() {
    auto ret = make_shared<MultiSystem>(name());
    for (auto& sys : m_sys_list) {
        ret->add(sys->clone());
    }
    ret->m_sub_init_cash = m_sub_init_cash;
    ret->m_adjust_cycle = m_adjust_cycle;
    ret->m_trade_on_close = m_trade_on_close;
    ret->m_sell_at_not_selected = m_sell_at_not_selected;
    if (getMM()) {
        ret->setMM(getMM()->clone());
    }
    if (m_se) {
        ret->m_se = m_se->clone();
    }
    // setMode 会同步设置 MM 的模式，须在 setMM 之后调用，确保克隆体保留 A/B 模式
    ret->setMode(m_mode);
    return ret;
}

void MultiSystem::run(const KData& kdata, bool reset, bool resetAll) {
    HKU_WARN_IF_RETURN(m_sys_list.empty(), void(), "No subsystem specified!");
    m_kdata = kdata;

    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    readyForRun();

    // 通知 SE 实际运行的系统列表（与原型映射），并启动其计算
    if (m_se) {
        m_se->calculate(m_sys_list, m_kdata.getQuery());
    }

    // 与单证券 System::run 一致的账户日期过滤：仅驱动 [账户初始化日, 账户最后成交日] 之后的 bar。
    // 目的：实盘每日以 BrokerTM 全量重放对齐时间轴时，跳过已在真实账户执行过的历史 bar，
    //       避免重复下单污染真实账户。对全新回测账户（lastDatetime == initDatetime == 时间轴起点）
    //       该过滤为空操作，不影响组合回测遍历全轴。
    Datetime tm_init_datetime = m_tm->initDatetime();
    Datetime tm_last_datetime = m_tm->lastDatetime();
    if (KQuery::getKTypeInSeconds(m_kdata.getQuery().kType()) >= 86400) {
        tm_init_datetime = tm_init_datetime.startOfDay();
        tm_last_datetime = tm_last_datetime.startOfDay();
    }

    // 聚合系统在完整对齐时间轴上驱动所有子系统
    size_t total = m_kdata.size();
    auto const* ks = m_kdata.data();
    for (size_t i = 0; i < total; ++i) {
        if (ks[i].datetime >= tm_init_datetime && ks[i].datetime >= tm_last_datetime) {
            runMoment(ks[i].datetime);
        }
    }
    m_calculated = true;
}

TradeSuggestionList MultiSystem::_toSuggestions(const SystemPtr& sys,
                                                const TradeRecordList& trades) const {
    TradeSuggestionList result;
    std::map<Stock, double> net;       // 净数量（正=买，负=卖）
    std::map<Stock, bool> is_clear;    // 是否整体清仓
    std::map<Stock, price_t> price;    // 计划价（取成交价）

    for (const auto& tr : trades) {
        if (tr.business == BUSINESS_INVALID) {
            continue;
        }
        if (tr.business == BUSINESS_BUY || tr.business == BUSINESS_BUY_SHORT) {
            net[tr.stock] += tr.number;
        } else if (tr.business == BUSINESS_SELL || tr.business == BUSINESS_SELL_SHORT) {
            net[tr.stock] -= tr.number;
            if (tr.number >= MAX_DOUBLE) {
                is_clear[tr.stock] = true;
            }
        }
        if (tr.realPrice > 0.0) {
            price[tr.stock] = tr.realPrice;
        }
    }

    for (auto& kv : net) {
        const Stock& stock = kv.first;
        double n = kv.second;
        if (stock.isNull() || n == 0.0) {
            continue;
        }
        TradeSuggestion s;
        s.stock = stock;
        s.sys = sys;
        s.number = std::fabs(n);
        s.plan_price = price[stock];
        s.plan_cash = s.number * s.plan_price;
        s.from = PART_SYSTEM;
        if (n > 0.0) {
            s.type = SuggestionType::BUY;
        } else {
            s.type = is_clear[stock] ? SuggestionType::CLEAR : SuggestionType::SELL;
        }
        result.push_back(s);
    }
    return result;
}

bool MultiSystem::_isAdjustDate() const {
    if (m_adjust_cycle <= 1) {
        return true;
    }
    return (m_close_day_index % static_cast<size_t>(m_adjust_cycle)) == 0;
}

void MultiSystem::_executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                                      KQuery::KType ktype, TradeRecordList& out_trades) {
    // 先卖后买，释放现金
    for (const auto& s : suggestions) {
        if (s.stock.isNull()) {
            continue;
        }
        if (s.type == SuggestionType::BUY) {
            continue;
        }
        // SELL：卖出建议数量（模式 A 已改写为「-父当前持仓」= 全平；模式 B 透传子指令）
        // CLEAR：全额退出
        double num = s.number;
        if (s.type == SuggestionType::CLEAR || num >= MAX_DOUBLE) {
            num = MAX_DOUBLE;
        } else if (num < 0.0) {
            num = -num;
        }
        if (num <= 0.0) {
            continue;
        }
        TradeRecord tr =
            m_tm->sell(date, s.stock, s.plan_price, num, 0.0, 0.0, s.plan_price, PART_SYSTEM,
                       "MultiSystem");
        if (!tr.isNull()) {
            out_trades.push_back(tr);
        }
    }
    for (const auto& s : suggestions) {
        if (s.stock.isNull()) {
            continue;
        }
        if (s.type != SuggestionType::BUY) {
            continue;
        }
        if (s.number <= 0.0 || s.plan_price <= 0.0) {
            continue;
        }
        double min_trade = s.stock.minTradeNumber();
        double qty = std::floor(s.number / min_trade) * min_trade;
        if (qty >= min_trade) {
            TradeRecord tr = m_tm->buy(date, s.stock, s.plan_price, qty, 0.0, 0.0, s.plan_price,
                                       PART_SYSTEM, "MultiSystem");
            if (!tr.isNull()) {
                out_trades.push_back(tr);
            }
        }
    }
}

MomentResult MultiSystem::runMoment(const Datetime& datetime) {
    MomentResult result;
    result.datetime = datetime;
    KQuery::KType ktype = m_kdata.getQuery().kType();

    result.funds_before_open = m_tm->getFunds(datetime, ktype);

    // 开盘阶段：驱动子系统兑现延迟请求，并收集开盘成交（供收盘阶段汇总为对上建议）
    MomentResult open_result = runMomentOnOpen(datetime);
    result.tradesOnOpen = open_result.tradesOnOpen;

    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    // 收盘阶段：驱动子系统生成信号，合并「开盘+收盘」成交汇总为建议，由父统一下单
    TradeRecordList executed = _closePhase(datetime);
    for (auto& tr : executed) {
        result.tradesOnClose.push_back(tr);
        m_trade_list.push_back(tr);
    }

    result.funds = m_tm->getFunds(datetime, ktype);
    return result;
}

MomentResult MultiSystem::runMomentOnOpen(const Datetime& datetime) {
    MomentResult result;
    result.datetime = datetime;
    // 父在开盘阶段统一处理退市标的：强制卖出父持仓
    TradeRecordList delist_trades = _forceSellDelisted(datetime);
    for (auto& tr : delist_trades) {
        result.tradesOnOpen.push_back(tr);
        m_trade_list.push_back(tr);
    }

    // 新的一天：清空并重建各子系统开盘成交缓冲（供本层收盘阶段汇总使用）。
    // 注意：子系统开盘成交发生在各自（虚拟）账户上，仅缓存进 m_open_trades 供本层收盘合并；
    // 【不计入】父自身的 tradesOnOpen —— 否则当本 MultiSystem 作为上层聚合的子系统时，上层会把这些
    // 孙系统原始开盘成交与本层收盘已净额化执行的成交重复计入（嵌套双计），导致对上建议方向/数量错误。
    // 父自身账户的开盘成交（如退市强平）已在上面并入 result.tradesOnOpen，符合 MomentResult 契约。
    m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
    m_open_trades_date = datetime;
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        MomentResult sub = m_sys_list[i]->runMomentOnOpen(datetime);
        for (auto& tr : sub.tradesOnOpen) {
            m_open_trades[i].push_back(tr);
        }
    }
    return result;
}

MomentResult MultiSystem::runMomentOnClose(const Datetime& datetime) {
    MomentResult result;
    result.datetime = datetime;
    KQuery::KType ktype = m_kdata.getQuery().kType();
    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    TradeRecordList executed = _closePhase(datetime);
    for (auto& tr : executed) {
        result.tradesOnClose.push_back(tr);
        m_trade_list.push_back(tr);
    }

    result.funds = m_tm->getFunds(datetime, ktype);
    return result;
}

TradeRecordList MultiSystem::_closePhase(const Datetime& datetime) {
    KQuery::KType ktype = m_kdata.getQuery().kType();
    TradeSuggestionList suggestions;
    SubSystemContextList contexts;

    // 防越界 + 防跨日残留：确保开盘缓冲与子系统数量对齐，且仅当缓冲属于当前交易日时才沿用。
    // 场景：实盘盘中若仅注册收盘驱动（当日未先调用 runMomentOnOpen）或刚经历 reset，
    //       m_open_trades 可能为空（下面按 [i] 索引会越界崩溃）或残留前一交易日开盘成交
    //       （与今日收盘建议重复合并 → 重复下单）。isNull() 前置短路，避免对 Null 调用 startOfDay()。
    if (m_open_trades.size() != m_sys_list.size() || m_open_trades_date.isNull() ||
        m_open_trades_date.startOfDay() != datetime.startOfDay()) {
        m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
        m_open_trades_date = datetime;
    }

    bool is_adjust = _isAdjustDate();

    // 调仓日 SE 选股：只对选中子系统收集建议；未选中按 sell_at_not_selected 清仓。
    // 非调仓日不启用 SE 过滤（各子系统照常运行）。
    std::set<System*> selected;
    if (m_se && is_adjust) {
        SystemWeightList sws = m_se->getSelected(datetime);
        for (auto& sw : sws) {
            if (sw.sys) {
                selected.insert(sw.sys.get());
            }
        }
    }

    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr sys = m_sys_list[i];
        if (m_se && is_adjust && selected.count(sys.get()) == 0) {
            // 未选中子系统：若父在其标的上持有仓位，则生成清仓建议
            if (m_sell_at_not_selected && !sys->getStock().isNull() && m_tm->have(sys->getStock())) {
                TradeSuggestion s;
                s.stock = sys->getStock();
                s.sys = sys;
                s.type = SuggestionType::CLEAR;
                s.plan_price = _getClosePrice(datetime, s.stock);
                s.number = m_tm->getPosition(datetime, s.stock).number;
                suggestions.push_back(s);
            }
            continue;
        }

        MomentResult sub = sys->runMomentOnClose(datetime);
        // 子系统决策可能体现在开盘成交（延迟买入）或收盘成交（立即买卖），
        // 合并后转译为对父建议（模式 A/B 共用此粘合剂）。
        TradeRecordList sub_trades = m_open_trades[i];
        sub_trades.insert(sub_trades.end(), sub.tradesOnClose.begin(), sub.tradesOnClose.end());
        TradeSuggestionList subsug = _toSuggestions(sys, sub_trades);
        for (auto& s : subsug) {
            suggestions.push_back(s);
        }
        SubSystemContext ctx;
        ctx.sys = sys;
        ctx.funds = sys->getTM()->getFunds(datetime, ktype);
        contexts.push_back(ctx);
    }

    m_close_day_index++;

    TradeRecordList executed;
    if (m_trade_on_close && is_adjust) {
        if (m_mode == "B") {
            // 模式 B：即使无交易建议也要运行 L1 产出下期额度（额度分配独立于建议），
            // L2 透传子系统真实指令；调仓日回写下期额度（滞后一期，额度穿透）。
            getMM()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
            if (!suggestions.empty()) {
                _executeSuggestions(datetime, suggestions, ktype, executed);
            }
            for (auto& ctx : contexts) {
                if (ctx.quota > 0.0) {
                    setSubSystemQuota(ctx.sys, datetime, ctx.quota);
                }
            }
        } else if (!suggestions.empty()) {
            // 模式 A：等权换算后父统一下单
            getMM()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
            _executeSuggestions(datetime, suggestions, ktype, executed);
        }
    }

    // 调仓换手率：成交金额 / 调仓前总资产（仅实际发生调仓成交的调仓日记录）
    if (is_adjust && m_trade_on_close && !executed.empty()) {
        double turnover_cash = 0.0;
        for (auto& tr : executed) {
            turnover_cash += std::fabs(tr.realPrice) * tr.number;
        }
        double assets = m_tm->getFunds(datetime, ktype).total_assets();
        m_adjust_turnover.emplace_back(datetime, assets > 0.0 ? turnover_cash / assets : 0.0);
    }

    // trace：输出调仓建议与成交
    if (getParam<bool>("trace")) {
        HKU_INFO("[{}] {} adjust suggestions={} executed={}", getPath(), name(), suggestions.size(),
                 executed.size());
        for (auto& s : suggestions) {
            const char* typ =
              s.type == SuggestionType::BUY ? "BUY" : (s.type == SuggestionType::CLEAR ? "CLEAR" : "SELL");
            HKU_INFO("[{}]   sug {} {} num={:.2f} price={:.3f}", getPath(), s.stock.market_code(), typ,
                     s.number, s.plan_price);
        }
    }

    m_last_suggestions = suggestions;
    return executed;
}

price_t MultiSystem::_getClosePrice(const Datetime& date, const Stock& stock) const {
    if (stock.isNull()) {
        return 0.0;
    }
    KData kdata = stock.getKData(m_kdata.getQuery());
    size_t pos = kdata.getPos(date);
    return pos == Null<size_t>() ? 0.0 : kdata.getKRecord(pos).closePrice;
}

TradeRecordList MultiSystem::_forceSellDelisted(const Datetime& date) {
    TradeRecordList result;
    if (!m_tm) {
        return result;
    }
    auto positions = m_tm->getPositionList();
    for (auto& pos : positions) {
        if (pos.stock.isNull()) {
            continue;
        }
        KData kdata = pos.stock.getKData(m_kdata.getQuery());
        if (kdata.empty()) {
            continue;
        }
        Datetime last_dt = kdata[kdata.size() - 1].datetime;
        if (last_dt == Null<Datetime>() || last_dt >= date) {
            continue;
        }
        // 标的最后交易日已过（退市）：以最后交易日收盘价强制清仓
        price_t price = kdata.getKRecord(kdata.size() - 1).closePrice;
        TradeRecord tr = m_tm->sell(date, pos.stock, price, MAX_DOUBLE, 0.0, 0.0, price, PART_SYSTEM,
                                    "DELIST");
        if (!tr.isNull()) {
            result.push_back(tr);
        }
    }
    return result;
}

void MultiSystem::setSubSystemQuota(const SYSPtr& sub_sys, const Datetime& date, price_t quota) {
    HKU_WARN_IF_RETURN(!sub_sys, void(), "Null subsystem!");
    TMPtr sub_tm = sub_sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sub_sys->name());
    HKU_WARN_IF_RETURN(quota <= 0.0, void(), "Invalid quota {} for subsystem {}!", quota,
                       sub_sys->name());

    // 把子系统的「总资产」调整到目标额度：
    //   - 配额增加：存入现金（checkin 差额）
    //   - 配额减少：提取现金（checkout 差额；现金不足时由子系统自行减仓，此处记录警告）
    // 聚合子系统（嵌套）同样通过调整其虚拟账户总资产，触发其内部分配（额度穿透）。
    FundsRecord funds = sub_tm->getFunds(date, KQuery::DAY);
    price_t diff = quota - funds.total_assets();
    if (diff > 0.0) {
        sub_tm->checkin(date, diff);
    } else if (diff < 0.0) {
        HKU_WARN_IF_RETURN(!sub_tm->checkout(date, -diff), void(),
                           "Quota reduction {} exceeds sub cash, subsystem must reduce position "
                           "itself! {}",
                           -diff, sub_sys->name());
    }
}

TradeRecord MultiSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(m_sys_list.empty(), ret, "No subsystem specified!");
    for (auto& sys : m_sys_list) {
        TradeRecord tr = sys->sellForceOnOpen(date, num, from);
        if (!tr.isNull()) {
            ret = tr;
            m_trade_list.push_back(tr);
        }
    }
    return ret;
}

TradeRecord MultiSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(m_sys_list.empty(), ret, "No subsystem specified!");
    for (auto& sys : m_sys_list) {
        TradeRecord tr = sys->sellForceOnClose(date, num, from);
        if (!tr.isNull()) {
            ret = tr;
            m_trade_list.push_back(tr);
        }
    }
    return ret;
}

void MultiSystem::clearDelayBuyRequest() {
    HKU_WARN_IF_RETURN(m_sys_list.empty(), void(), "No subsystem specified!");
    for (auto& sys : m_sys_list) {
        sys->clearDelayBuyRequest();
    }
    m_buyRequestList.clear();
}

TradeRecord MultiSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(m_sys_list.empty(), ret, "No subsystem specified!");
    for (auto& sys : m_sys_list) {
        TradeRecord tr = sys->pfProcessDelaySellRequest(date);
        if (!tr.isNull()) {
            ret = tr;
            m_trade_list.push_back(tr);
        }
    }
    return ret;
}

}  // namespace hku
