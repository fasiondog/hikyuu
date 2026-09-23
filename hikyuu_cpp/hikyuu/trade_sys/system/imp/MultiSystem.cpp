/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  Recursive combination refactoring: the aggregate trading system (portfolio backtesting)
 *  Stage 3: dual modes (A/B) + arbitrary nesting + MM L1/L2/L3 + rebalancing cycle + hierarchy path.
 */

#include "MultiSystem.h"

#include <map>
#include <cmath>
#include <set>
#include <unordered_map>

#include "../../selector/SelectorBase.h"
#include "../../../utilities/arithmetic.h"
#include "../../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiSystem)
#endif

namespace hku {

static const string s_default_af_mode = "A";

const string& MultiSystem::getMode() const {
    return m_af ? m_af->getMode() : s_default_af_mode;
}

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

    // The aggregate form does not validate its own single-security parts such as SG/MM/ST (they belong to every sub-system);
    // it only prepares the running environment, the sub-system parts are completed by the readyForRun of every sub-system in the validation loop.
    m_close_day_index = 0;
    if (m_path.empty()) {
        m_path = name();
    }

    // Create an independent virtual account for every sub-system (mode A: a fixed shadow account; mode B: the quota allocated by the parent)
    // The sub-systems keep their own SG/MM/EV/CN/ST/TP/PG/SP (the strategies of their own independent securities), only the accounts are isolated.
    for (auto& sys : m_sys_list) {
        TMPtr sub_tm = crtTM(m_tm->initDatetime(), m_sub_init_cash, TC_Zero(), "TM_SUB");
        sys->setTM(sub_tm);
        sys->setParam<bool>("shared_tm", false);
        // The hierarchy path is written recursively
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
    m_sub_funds_before.clear();
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
    m_sub_funds_before.clear();
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
    ret->m_date_axis = m_date_axis;  // The fixed time axis is copied with the configuration (the axis-mode parameter is copied by System::clone)
    ret->m_adjust_dates = m_adjust_dates;  // The external rebalancing day table is copied with the configuration
    if (getMM()) {
        ret->setMM(getMM()->clone());
    }
    // The AF clone has already copied m_mode, no additional mode synchronization is needed
    if (m_af) {
        ret->setAF(m_af->clone());
    }
    if (m_se) {
        ret->m_se = m_se->clone();
    }
    return ret;
}

void MultiSystem::setAxisMode(const string& mode) {
    if (mode != "kdata" && mode != "calendar") {
        HKU_WARN("Invalid axis-mode: {}, only \"kdata\" / \"calendar\" supported, fallback to \"kdata\"! [{}]",
                 mode, name());
        setParam<string>("axis-mode", "kdata");
        return;
    }
    setParam<string>("axis-mode", mode);
}

void MultiSystem::setAdjustDates(const DatetimeList& dates) {
    m_adjust_dates.clear();
    for (const auto& date : dates) {
        if (!date.isNull()) {
            m_adjust_dates.insert(date.startOfDay());
        }
    }
}

void MultiSystem::setAdjustMode(const string& mode) {
    string m = mode;
    to_lower(m);
    if (m != "query" && m != "day" && m != "week" && m != "month" && m != "quarter" &&
        m != "year") {
        HKU_WARN("Invalid adjust-mode: {}, only query/day/week/month/quarter/year supported, "
                 "fallback to \"query\"! [{}]",
                 mode, name());
        setParam<string>("adjust-mode", "query");
        return;
    }
    setParam<string>("adjust-mode", m);
}

void MultiSystem::_expandAdjustDates(const DatetimeList& axis) {
    m_auto_adjust_dates.clear();
    // The external explicit injection takes precedence, no auto expansion (to keep the high priority of setAdjustDates)
    if (!m_adjust_dates.empty() || axis.empty()) {
        return;
    }
    string mode = getAdjustMode();
    to_lower(mode);
    if (mode == "query" || mode == "day") {
        return;  // Continue the close-day counting judgment of m_adjust_cycle
    }
    DatetimeList expanded = calcAdjustDates(axis, mode, m_adjust_cycle, getDelayToTradingDay());
    for (const auto& d : expanded) {
        m_auto_adjust_dates.insert(d.startOfDay());
    }
    if (getParam<bool>("trace")) {
        HKU_INFO("[{}] adjust-mode={} expand adjust dates: {} (axis={})", name(), mode,
                 m_auto_adjust_dates.size(), axis.size());
    }
}

DatetimeList MultiSystem::calcAdjustDates(const DatetimeList& dates, const string& mode,
                                          int adjust_cycle, bool delay_to_trading_day) {
    std::set<Datetime> result;
    if (dates.empty()) {
        return DatetimeList();
    }
    // Strictly restrict mode: only week/month/quarter/year are valid, the others (query/day/invalid values) return empty
    string m = mode;
    to_lower(m);
    if (m != "week" && m != "month" && m != "quarter" && m != "year") {
        return DatetimeList();
    }
    const size_t total = dates.size();
    const int cycle = adjust_cycle > 0 ? adjust_cycle : 1;

    if (delay_to_trading_day) {
        // The postponement semantics: when the target day is not a trading day, it is postponed to the first trading day within the current cycle (consistent with master Portfolio).
        // emitted records the "theoretical rebalancing day" already hit, only one hit per cycle.
        std::set<Datetime> emitted;
        for (size_t i = 0; i < total; ++i) {
            const Datetime& date = dates[i];
            Datetime adjust_date;
            if (m == "week") {
                adjust_date = date.startOfWeek() + Days(cycle - 1);
            } else if (m == "month") {
                adjust_date = date.startOfMonth() + Days(cycle - 1);
            } else if (m == "quarter") {
                adjust_date = date.startOfQuarter() + Days(cycle - 1);
            } else {  // year
                adjust_date = date.startOfYear() + Days(cycle - 1);
            }
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                emitted.emplace(adjust_date);
            } else if (emitted.find(adjust_date) == emitted.end() && date > adjust_date) {
                adjust = true;
                emitted.emplace(adjust_date);
            }
            if (adjust) {
                result.insert(date);
            }
        }
    } else {
        for (size_t i = 0; i < total; ++i) {
            const Datetime& date = dates[i];
            bool adjust = false;
            if (m == "week") {
                adjust = (date.dayOfWeek() == cycle);  // The cycle-th day of every week (0=Sunday, 1=Monday ... 6=Saturday)
            } else if (m == "month" || m == "quarter") {
                adjust = (date.day() == cycle);  // The cycle-th day of every month/quarter
            } else {                             // year
                adjust = (date.dayOfYear() == cycle);  // The cycle-th day of every year
            }
            if (adjust) {
                result.insert(date);
            }
        }
    }
    return DatetimeList(result.begin(), result.end());
}

void MultiSystem::run(const KData& kdata, bool reset, bool resetAll) {
    // The driving axis selection (the axis-mode parameter, see MultiSystem.h):
    //   "kdata"    -- the date sequence of the input KData is used as the axis (the default, keeping the existing behavior)
    //   "calendar" -- the fixed date table (e.g. the market-wide trading calendar) injected by setDateAxis() is used as the axis;
    //                 the input KData degenerates into the query/ktype and the price query context (runMoment/_closePhase only take its query)
    string axis_mode = tryGetParam<string>("axis-mode", "kdata");
    bool use_calendar_axis = (axis_mode == "calendar");
    if (use_calendar_axis && m_date_axis.empty()) {
        HKU_WARN("axis-mode=calendar but date axis is empty, fallback to kdata axis! [{}]", name());
        use_calendar_axis = false;
    }
    _runAxis(kdata, use_calendar_axis ? &m_date_axis : nullptr, reset, resetAll);
}

Stock MultiSystem::_findStock(const SystemPtr& sys) {
    HKU_IF_RETURN(!sys, Stock());
    Stock stk = sys->getStock();
    HKU_IF_RETURN(!stk.isNull(), stk);
    for (const auto& sub : sys->getSubSystemList()) {
        stk = _findStock(sub);
        HKU_IF_RETURN(!stk.isNull(), stk);
    }
    return Stock();
}

void MultiSystem::run(const KQuery& query, bool reset, bool resetAll) {
    // master compatibility overload: equivalent to Portfolio::run(query), it uses the market trading calendar as the driving axis (see design.md §4.5)
    auto& sm = StockManager::instance();

    // Consistent with master: when ktype is not the daily line, it is only allowed when adjust-mode is query/day (the calendar axis is a daily line sequence)
    string mode = getAdjustMode();
    to_lower(mode);
    HKU_CHECK(mode == "query" || mode == "day" || query.kType() == KQuery::DAY,
              "The kType of query must be DAY when adjust-mode is not \"query\"! [{}]", name());

    // The driving axis: the explicitly injected fixed time axis takes precedence (respecting the injection of the axis-mode=calendar users), otherwise the market trading calendar is used
    DatetimeList dates = (getAxisMode() == "calendar" && !m_date_axis.empty())
                           ? m_date_axis
                           : sm.getTradingCalendar(query);
    HKU_WARN_IF_RETURN(dates.empty(), void(), "No trading date in the query range! [{}]", name());

    // The context KData (only carrying the query/ktype and the price query context):
    //   its own instrument (explicitly set) -> the first (recursive) sub-system instrument -> the calendar benchmark index
    Stock ref_stk = getStock();
    for (size_t i = 0; i < m_sys_list.size() && ref_stk.isNull(); ++i) {
        ref_stk = _findStock(m_sys_list[i]);
    }
    if (ref_stk.isNull()) {
        MarketInfo market_info = sm.getMarketInfo("SH");
        ref_stk = sm.getStock(market_info.market() + market_info.code());
    }
    HKU_WARN_IF_RETURN(ref_stk.isNull(), void(), "No stock as price context! [{}]", name());

    _runAxis(ref_stk.getKData(query), &dates, reset, resetAll);
}

void MultiSystem::_runAxis(const KData& kdata, const DatetimeList* axis, bool reset, bool resetAll) {
    HKU_WARN_IF_RETURN(m_sys_list.empty(), void(), "No subsystem specified!");
    m_kdata = kdata;

    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    readyForRun();

    // Notify SE of the actually running system list (mapped with the prototypes), and start its calculation
    if (m_se) {
        m_se->calculate(m_sys_list, m_kdata.getQuery());
    }

    // The account date filtering consistent with the single-security System::run: only the bars after [the account initialization day, the account last trade day] are driven.
    // Purpose: when the live trading replays with the BrokerTM full alignment of the time axis daily, skip the historical bars already executed on the real account,
    //       to avoid the duplicate orders polluting the real account. For a brand-new backtesting account (lastDatetime == initDatetime == the start of the time axis)
    //       this filtering is a no-op, it does not affect the portfolio backtesting traversing the whole axis.
    Datetime tm_init_datetime = m_tm->initDatetime();
    Datetime tm_last_datetime = m_tm->lastDatetime();
    if (KQuery::getKTypeInSeconds(m_kdata.getQuery().kType()) >= 86400) {
        tm_init_datetime = tm_init_datetime.startOfDay();
        tm_last_datetime = tm_last_datetime.startOfDay();
    }

    // v5: adjust-mode internalization -- when it is not query/day, expand the rebalancing day table on the "driving axis" (design.md §4.3).
    // The expansion only depends on the driving axis itself, decoupled from the driving loop; the external setAdjustDates() injection takes precedence.
    _expandAdjustDates(axis ? *axis : m_kdata.getDatetimeList());

    if (axis) {
        // Driven by the fixed time axis: the dates on the axis may not exist in the input KData (the suspended/non-trading days do not constitute a gap)
        for (const auto& dt : *axis) {
            if (dt >= tm_init_datetime && dt >= tm_last_datetime) {
                runMoment(dt);
            }
        }
    } else {
        // The aggregate system drives all the sub-systems on the fully aligned time axis
        size_t total = m_kdata.size();
        auto const* ks = m_kdata.data();
        for (size_t i = 0; i < total; ++i) {
            if (ks[i].datetime >= tm_init_datetime && ks[i].datetime >= tm_last_datetime) {
                runMoment(ks[i].datetime);
            }
        }
    }
    m_calculated = true;
}

TradeSuggestionList MultiSystem::_toSuggestions(const SystemPtr& sys, const TradeRecordList& trades,
                                                const FundsRecord& funds_before) const {
    TradeSuggestionList result;
    // The "before-trade" fund benchmark of the sub-system (to prevent division by zero): used to calculate the three ratios of the suggestion (the complete semantic pass-through of the design 8.2/8.3)
    double base_assets = funds_before.total_assets();
    double base_cash = funds_before.cash;
    std::map<Stock, double> net;       // The net quantity (positive=buy, negative=sell)
    std::map<Stock, bool> is_clear;    // Whether to liquidate the whole position
    std::map<Stock, price_t> price;    // The planned price (the traded price)

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
        // The three ratios: the sub-system before-trade funds are used as the denominator. The parent MM maps them into the parent real assets by the ratios (mode A).
        if (base_assets > 0.0) {
            s.assets_ratio = s.plan_cash / base_assets;
            s.target_position_ratio = s.assets_ratio;  // Approximation: the target position ratio when a single trade builds the position from 0
        }
        if (base_cash > 0.0) {
            s.cash_ratio = s.plan_cash / base_cash;
        }
        result.push_back(s);
    }
    return result;
}

bool MultiSystem::_isAdjustDate(const Datetime& date) const {
    // The external rebalancing day table takes precedence: the rebalancing is only performed on the dates hitting the table (used when PF maps the master adjust_mode / delay_to_trading_day)
    if (!date.isNull()) {
        if (!m_adjust_dates.empty()) {
            return m_adjust_dates.find(date.startOfDay()) != m_adjust_dates.end();
        }
        // The rebalancing day table auto-expanded by adjust-mode ∈ {week,month,quarter,year}
        if (!m_auto_adjust_dates.empty()) {
            return m_auto_adjust_dates.find(date.startOfDay()) != m_auto_adjust_dates.end();
        }
    }
    // Fallback: rebalance every m_adjust_cycle close days
    if (m_adjust_cycle <= 1) {
        return true;
    }
    return (m_close_day_index % static_cast<size_t>(m_adjust_cycle)) == 0;
}

void MultiSystem::_executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                                      KQuery::KType ktype, TradeRecordList& out_trades) {
    // Sell first then buy, to release the cash
    for (const auto& s : suggestions) {
        if (s.stock.isNull()) {
            continue;
        }
        if (s.type == SuggestionType::BUY) {
            continue;
        }
        // SELL: sell the suggested quantity (in mode A it is rewritten to "-the parent current position" = full close; in mode B the sub-instruction is passed through)
        // CLEAR: exit the whole position
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

    // The open stage: drive the sub-systems to fulfill the delayed requests, and collect the open trades (used by the close stage to aggregate into the parent suggestions)
    MomentResult open_result = runMomentOnOpen(datetime);
    result.tradesOnOpen = open_result.tradesOnOpen;

    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    // The close stage: drive the sub-systems to generate signals, merge the "open+close" trades and aggregate them into the suggestions, the parent orders uniformly
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
    // The parent handles the delisted instruments uniformly at the open stage: force selling the parent holdings
    TradeRecordList delist_trades = _forceSellDelisted(datetime);
    for (auto& tr : delist_trades) {
        result.tradesOnOpen.push_back(tr);
        m_trade_list.push_back(tr);
    }

    // A new day: clear and rebuild the open trade buffer of every sub-system (used by the close stage of this layer for the aggregation).
    // Note: the open trades of the sub-systems happen on their own (virtual) accounts, they are only cached into m_open_trades for the close merge of this layer;
    // they are [not counted] into the tradesOnOpen of the parent itself -- otherwise when this MultiSystem is a sub-system of an upper aggregate, the upper layer would count
    // these grand-system raw open trades and the net trades already executed by this layer's close together (nested double counting), causing the wrong direction/quantity of the parent suggestion.
    // The open trades of the parent's own account (e.g. the delisting forced liquidation) have been merged into result.tradesOnOpen above, conforming to the MomentResult contract.
    m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
    m_sub_funds_before.assign(m_sys_list.size(), FundsRecord{});
    m_open_trades_date = datetime;
    KQuery::KType ktype = m_kdata.getQuery().kType();
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        // The "before-trade" fund snapshot of the sub-system on that day (before the open drive), used by the close stage _toSuggestions to calculate the three ratios
        if (m_sys_list[i]->getTM()) {
            m_sub_funds_before[i] = m_sys_list[i]->getTM()->getFunds(datetime, ktype);
        }
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

    // Prevent out-of-bounds + prevent cross-day residue: ensure the open buffer is aligned with the sub-system quantity, and it is only reused when the buffer belongs to the current trading day.
    // Scenario: if only the close drive is registered during the live trading (runMomentOnOpen is not called first on that day) or it has just gone through reset,
    //       m_open_trades may be empty (the [i] indexing below would crash out-of-bounds) or hold the open trades of the previous trading day
    //       (duplicate merging with the today close suggestions -> duplicate orders). The isNull() pre-short-circuit avoids calling startOfDay() on Null.
    if (m_open_trades.size() != m_sys_list.size() || m_sub_funds_before.size() != m_sys_list.size() ||
        m_open_trades_date.isNull() || m_open_trades_date.startOfDay() != datetime.startOfDay()) {
        m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
        m_sub_funds_before.assign(m_sys_list.size(), FundsRecord{});
        m_open_trades_date = datetime;
        // The before-trade snapshot is missing (e.g. only the close drive is registered during the session, runMomentOnOpen is not called first): use the current funds of the sub-system as the fallback, to ensure the ratio denominator is not empty
        KQuery::KType kt = m_kdata.getQuery().kType();
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            if (m_sys_list[i]->getTM()) {
                m_sub_funds_before[i] = m_sys_list[i]->getTM()->getFunds(datetime, kt);
            }
        }
    }

    bool is_adjust = _isAdjustDate(datetime);

    // The SE stock selection on the rebalancing day: only collect the suggestions of the selected sub-systems; the unselected ones are liquidated by sell_at_not_selected.
    // The SE filtering is not enabled on the non-rebalancing days (every sub-system runs normally).
    std::set<System*> selected;
    std::unordered_map<System*, double> se_scores;  // v5: the SE scores, used by AF_MultiFactor etc. to take the scores as the weights
    if (m_se && is_adjust) {
        SystemWeightList sws = m_se->getSelected(datetime);
        for (auto& sw : sws) {
            if (sw.sys) {
                selected.insert(sw.sys.get());
                se_scores[sw.sys.get()] = sw.weight;
            }
        }
    }

    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr sys = m_sys_list[i];
        if (m_se && is_adjust && selected.count(sys.get()) == 0) {
            // The unselected sub-system: if the parent holds a position on its instrument, generate a liquidation suggestion
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
        // The sub-system decision may be reflected in the open trade (the delayed buy) or the close trade (the immediate buy/sell),
        // it is merged and translated into the parent suggestion (mode A/B share this glue).
        TradeRecordList sub_trades = m_open_trades[i];
        sub_trades.insert(sub_trades.end(), sub.tradesOnClose.begin(), sub.tradesOnClose.end());
        TradeSuggestionList subsug = _toSuggestions(sys, sub_trades, m_sub_funds_before[i]);
        for (auto& s : subsug) {
            suggestions.push_back(s);
        }
        SubSystemContext ctx;
        ctx.sys = sys;
        ctx.funds = sys->getTM()->getFunds(datetime, ktype);
        // v5: backfill the SE score (0 on the non-rebalancing days / for the unselected ones), used by AF_MultiFactor etc. to take the scores as the weights
        auto score_it = se_scores.find(sys.get());
        if (score_it != se_scores.end()) {
            ctx.score = score_it->second;
        }
        contexts.push_back(ctx);
    }

    m_close_day_index++;

    TradeRecordList executed;
    if (m_trade_on_close && is_adjust) {
        if (getMode() == "B") {
            // Mode B: even without a trade suggestion, run L1 to produce the next-period quota (the quota allocation is independent of the suggestions),
            // L2 passes through the real instruction of the sub-system; the next-period quota is written back on the rebalancing day (lagging one period behind, quota penetration).
            getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
            if (!suggestions.empty()) {
                _executeSuggestions(datetime, suggestions, ktype, executed);
            }
            for (auto& ctx : contexts) {
                if (ctx.quota > 0.0) {
                    setSubSystemQuota(ctx.sys, datetime, ctx.quota);
                }
            }
        } else if (!suggestions.empty()) {
            // Mode A: after the L2 conversion of AF, the parent orders uniformly
            getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
            _executeSuggestions(datetime, suggestions, ktype, executed);
        }
    }

    // The rebalancing turnover rate: the turnover amount / the total assets before rebalancing (only recorded on the rebalancing days with actual rebalancing trades)
    if (is_adjust && m_trade_on_close && !executed.empty()) {
        double turnover_cash = 0.0;
        for (auto& tr : executed) {
            turnover_cash += std::fabs(tr.realPrice) * tr.number;
        }
        double assets = m_tm->getFunds(datetime, ktype).total_assets();
        m_adjust_turnover.emplace_back(datetime, assets > 0.0 ? turnover_cash / assets : 0.0);
    }

    // trace: output the rebalancing suggestions and trades
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
        // The last trading day of the instrument has passed (delisting): force liquidation at the close price of the last trading day
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

    // Adjust the "total assets" of the sub-system to the target quota:
    //   - Quota increase: deposit the cash (checkin the difference)
    //   - Quota decrease: withdraw the cash (checkout the difference; when the cash is insufficient the sub-system reduces the position itself, a warning is recorded here)
    // The aggregate sub-system (nested) also triggers its internal allocation by adjusting the total assets of its virtual account (quota penetration).
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
