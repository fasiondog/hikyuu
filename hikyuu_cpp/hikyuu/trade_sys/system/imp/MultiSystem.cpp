/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  Recursive combination refactoring: the aggregate trading system (portfolio backtesting)
 *  Dual modes (A/B) + arbitrary nesting + MM L1/L2/L3 + rebalancing cycle + hierarchy path.
 */

#include "MultiSystem.h"

#include <map>
#include <cmath>
#include <mutex>
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
    HKU_WARN_IF_RETURN(_subtreeContains(sys, this), void(), "Cyclic reference detected! {}",
                       name());
    for (auto& s : m_sys_list) {
        if (s.get() == sys.get()) {
            HKU_WARN("Subsystem already exists, ignored! {}", name());
            return;
        }
    }
    m_sys_list.push_back(sys);
    m_sub_index[sys.get()] = m_sys_list.size() - 1;
}

void MultiSystem::readyForRun() {
    HKU_CHECK(m_tm, "Not setTradeManager! {}", name());
    // The SE-provided prototypes are materialized here as well, so that an aggregate configured
    // only with an SE can be prepared for running (the legacy Portfolio usage and the live trading
    // entry, which calls readyForRun before the first run)
    _adoptSEProtos();
    HKU_CHECK(!m_sys_list.empty(), "No subsystem specified! {}", name());

    // The aggregate form does not validate its own single-security parts such as SG/MM/ST (they
    // belong to every sub-system); it only prepares the running environment, the sub-system parts
    // are completed by the readyForRun of every sub-system in the validation loop.
    // Rebuild the O(1) slot index (the list constructors bypass add()); NOTE: the close-day
    // counter is deliberately NOT reset here -- this runs on every live replay day as well, and
    // resetting it would turn every day into a rebalancing day (the counter restarts only via
    // reset / forceResetAll, see _reset).
    m_sub_index.clear();
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        m_sub_index[m_sys_list[i].get()] = i;
    }
    if (m_path.empty()) {
        m_path = name();
    }

    // The account form of the sub-systems is decided by the running mode: mode A gives a fixed
    // shadow account, mode B gives a quota account starting from zero (both isolated from the real
    // funds of the parent); mode C shares the real account of the parent itself, so no shadow
    // account is created at all. The sub-systems keep their own
    // SG/MM/EV/CN/ST/TP/PG/SP (the strategies of their own independent securities).
    const bool mode_c = (getMode() == "C");
    for (auto& sys : m_sys_list) {
        if (mode_c) {
            // Mode C (the legacy Portfolio behavior): the sub-system trades directly on the real
            // account of the parent and is sized by its own MM.
            // @note shared_tm can only be set here, after the sub-systems have been adopted:
            //       SelectorBase::addStock rejects a prototype carrying shared_tm, so it must not
            //       be set earlier at the factory / the SE construction stage.
            sys->setTM(m_tm);
            sys->setParam<bool>("shared_tm", true);
            // Restore the delay mapping of the legacy Portfolio: when the rebalancing is executed
            // at the close the orders of the sub-systems must not be delayed, otherwise the delayed
            // requests get dropped by the SE buy gate on the next rebalancing day (the reason why
            // the refactored engine produced no trade at all for the existing strategies).
            const bool delay = !m_trade_on_close;
            sys->setParam<bool>("buy_delay", delay);
            sys->setParam<bool>("sell_delay", delay);
        } else {
            // The shadow account is created only once: re-creating it would wipe the sub-account
            // state of the reset=false live replay (the pending delayed requests, the mode B quota)
            // Mode B (master SimplePortfolio compatibility): the sub-account starts from zero and
            // is injected with the exact quota on every rebalancing day (the cost function follows
            // the parent account, so the shadow book keeps in step with the parent trades).
            // Mode A (signal aggregation): the fixed shadow cash (pure signal source).
            if (!m_shadow_sys.count(sys.get())) {
                price_t init_cash = (getMode() == "B") ? 0.0 : m_sub_init_cash;
                TMPtr sub_tm = crtTM(m_tm->initDatetime(), init_cash, m_tm->costFunc(), "TM_SUB");
                sys->setTM(sub_tm);
                m_shadow_sys.insert(sys.get());
            }
            sys->setParam<bool>("shared_tm", false);
        }
        // The hierarchy path is written recursively
        sys->setPath(m_path + "/" + sys->name());
        if (sys->isComposite()) {
            // A nested aggregate has no instrument of its own and its own run() is never called on
            // that path (the parent drives it bar by bar), so it inherits the driving KData of the
            // parent: otherwise its price queries and its own SE would be left with no time axis at
            // all and the nested layer could never trade
            auto inner = std::dynamic_pointer_cast<MultiSystem>(sys);
            if (inner && !m_kdata.empty()) {
                inner->m_kdata = m_kdata;
            }
        } else if (sys->getTO().empty()) {
            // master compatibility: when the sub-system has no trading object but its instrument is
            // set, build the TO with the query of this run (SimplePortfolio did the same for the
            // real systems cloned from the proto systems)
            Stock stk = sys->getStock();
            if (!stk.isNull() && !m_kdata.empty()) {
                sys->setTO(stk.getKData(m_kdata.getQuery()));
            } else {
                HKU_WARN("Subsystem {} has no trading object (setTO), it will run with no trades!",
                         sys->name());
            }
        } else {
            // The reset clears the internal state of the SG and other components, so the TO must
            // be reset to trigger the components to recalculate with the current TO (when
            // reset=false is replayed in live trading, it short-circuits via m_calculated and
            // causes no repeated overhead)
            sys->setTO(sys->getTO());
        }
        sys->readyForRun();
    }

    // Notify SE of the actually running system list and start its calculation. Apart from _runAxis
    // this is also needed here: an aggregate driven by its parent (the nesting case) never runs its
    // own _runAxis, so its SE would never learn its sub-systems and nothing would ever be admitted.
    // The internal m_calculated + query guard makes the repeated call free of cost.
    if (m_se && !m_sys_list.empty() && !m_kdata.empty()) {
        m_se->calculate(m_sys_list, m_kdata.getQuery());
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
    m_pending_suggestions.clear();
    m_open_pending_suggestions.clear();
    m_kdata_cache.clear();
    // mode C: the running pool and the force-sell pool are runtime states, dropped on reset
    m_running_set.clear();
    m_running_order.clear();
    m_force_sell_list.clear();
    m_signal_reset_date = Null<Datetime>();
    // The rebalancing cycle restarts with a fresh run; kept across the live reset=false replays
    m_close_day_index = 0;
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
    m_pending_suggestions.clear();
    m_open_pending_suggestions.clear();
    m_kdata_cache.clear();
    // mode C: the running pool and the force-sell pool are runtime states, dropped on reset
    m_running_set.clear();
    m_running_order.clear();
    m_force_sell_list.clear();
    m_signal_reset_date = Null<Datetime>();
    // The rebalancing cycle restarts with a fresh run; kept across the live reset=false replays
    m_close_day_index = 0;
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
    ret->m_path = m_path;            // The hierarchy path is copied with the configuration
    ret->m_date_axis = m_date_axis;  // The fixed time axis is copied with the configuration (the
                                     // axis-mode parameter is copied by System::clone)
    ret->m_adjust_dates =
      m_adjust_dates;  // The external rebalancing day table is copied with the configuration
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
    // m_shadow_sys / m_running_set / m_running_order / m_force_sell_list / m_sub_index /
    // m_pending_suggestions / m_open_pending_suggestions / m_kdata_cache are the runtime states,
    // not copied
    return ret;
}

void MultiSystem::setAxisMode(const string& mode) {
    if (mode != "kdata" && mode != "calendar") {
        HKU_WARN(
          "Invalid axis-mode: {}, only \"kdata\" / \"calendar\" supported, fallback to \"kdata\"! "
          "[{}]",
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
        HKU_WARN(
          "Invalid adjust-mode: {}, only query/day/week/month/quarter/year supported, "
          "fallback to \"query\"! [{}]",
          mode, name());
        setParam<string>("adjust-mode", "query");
        return;
    }
    setParam<string>("adjust-mode", m);
}

void MultiSystem::_expandAdjustDates(const DatetimeList& axis) {
    m_auto_adjust_dates.clear();
    // The external explicit injection takes precedence, no auto expansion (to keep the high
    // priority of setAdjustDates)
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

void MultiSystem::_buildCycleEnds(const DatetimeList& axis) {
    m_cycle_ends.clear();
    HKU_IF_RETURN(axis.empty(), void());

    // The rebalancing-day sequence: the explicit (setAdjustDates) or auto-expanded
    // (adjust-mode) table intersected with the axis takes precedence; otherwise fall back to the
    // close-day counting of m_adjust_cycle (consistent with _isAdjustDate)
    DatetimeList adjust_dates;
    const std::set<Datetime>* table =
      !m_adjust_dates.empty() ? &m_adjust_dates
                              : (!m_auto_adjust_dates.empty() ? &m_auto_adjust_dates : nullptr);
    if (table) {
        for (const auto& d : axis) {
            if (table->find(d.startOfDay()) != table->end()) {
                adjust_dates.emplace_back(d);
            }
        }
    } else {
        const int cycle = m_adjust_cycle > 0 ? m_adjust_cycle : 1;
        for (size_t i = 0; i < axis.size(); i += cycle) {
            adjust_dates.emplace_back(axis[i]);
        }
    }

    // The cycle end of a rebalancing day is the next rebalancing day; the last one runs to the end
    // of the axis (aligned with master Portfolio::m_cycle_end_dates)
    const size_t total = adjust_dates.size();
    for (size_t i = 0; i < total; ++i) {
        m_cycle_ends[adjust_dates[i].startOfDay()] =
          i + 1 < total ? adjust_dates[i + 1] : axis.back() + Minutes(1);
    }
}

Datetime MultiSystem::_getNextCycleEnd(const Datetime& date) const {
    auto iter = m_cycle_ends.find(date.startOfDay());
    return iter != m_cycle_ends.end() ? iter->second : date + Days(1);
}

void MultiSystem::_reduceSubSystemToQuota(const SystemPtr& sys, const Datetime& date, price_t quota,
                                          KQuery::KType ktype, TradeRecordList& out_executed) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    // The sub cash has been recycled before, so the sub total here is the holding market value
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t sub_total = funds.total_assets() - funds.cash;
    if (sub_total <= quota || quota <= 0.0) {
        return;
    }

    // Above the quota: reduce the position by the over quota market value (the master algorithm)
    price_t need_back_funds = sub_total - quota;
    Stock stock = sys->getStock();
    if (stock.isNull()) {
        return;
    }
    price_t last_close_price = stock.getMarketValue(date, ktype);
    if (last_close_price <= 0.0) {
        return;  // The security is invalid (e.g. delisted), nothing to do
    }
    double min_num = stock.minTradeNumber();
    if (min_num <= 0.0) {
        return;
    }
    PositionRecord position = sub_tm->getPosition(date, stock);
    double hold_num = position.number;
    if (hold_num <= 0.0) {
        return;
    }
    double need_back_num =
      static_cast<double>(static_cast<int64_t>(need_back_funds / last_close_price / min_num)) *
      min_num;
    if (hold_num - need_back_num < min_num) {
        need_back_num = hold_num;
    }
    if (need_back_num <= 0.0) {
        return;
    }

    price_t close_price = _getClosePrice(date, stock);
    if (close_price <= 0.0) {
        return;
    }

    // The sub-system sells on its own book, and the parent sells the same quantity immediately
    // (the master AF executed the reduction inside the sub account and recorded it in the total
    // account), so the freed cash is already in the parent pool when the injection phase runs
    TradeRecord tr = sys->sellForceOnClose(date, need_back_num, PART_PORTFOLIO);
    if (!tr.isNull()) {
        FundsRecord f2 = sub_tm->getFunds(date, ktype);
        if (f2.cash > 0.0) {
            sub_tm->checkout(date, f2.cash);
        }
    }
    TradeRecord parent_tr = m_tm->sell(date, stock, close_price, need_back_num, 0.0, 0.0,
                                       close_price, PART_PORTFOLIO, "MultiSystem");
    if (!parent_tr.isNull()) {
        out_executed.push_back(parent_tr);
    }
}

void MultiSystem::_injectSubSystemGap(const SystemPtr& sys, const Datetime& date, price_t quota,
                                      KQuery::KType ktype) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t sub_total = funds.total_assets() - funds.cash;
    if (sub_total >= quota || quota <= 0.0) {
        return;
    }

    // Below the quota: inject the cash gap (limited by the free cash of the parent pool)
    price_t diff = quota - sub_total;
    price_t parent_cash = m_tm->getFunds(date, ktype).cash;
    if (diff > parent_cash) {
        diff = parent_cash;
    }
    if (diff > 0.0) {
        sub_tm->checkin(date, diff);
    }
}

void MultiSystem::_clearSubSystem(const SystemPtr& sys, const Datetime& date, KQuery::KType ktype) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    Stock stock = sys->getStock();
    if (!stock.isNull()) {
        PositionRecord position = sub_tm->getPosition(date, stock);
        if (position.number > 0.0 && _getClosePrice(date, stock) > 0.0) {
            sys->sellForceOnClose(date, MAX_DOUBLE, PART_PORTFOLIO);
        }
    }
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    if (funds.cash > 0.0) {
        sub_tm->checkout(date, funds.cash);
    }
}

void MultiSystem::_resetSubSystemSignalCash(const SystemPtr& sys, const Datetime& date,
                                            KQuery::KType ktype) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    // Clear the shadow holding and the cash (bookkeeping only: the parent account position is
    // managed by the L2 target conversion, the shadow is a pure signal source), then inject the
    // initial signal cash again, so the sub-system can submit a fresh position intent
    _clearSubSystem(sys, date, ktype);
    if (m_sub_init_cash > 0.0) {
        sub_tm->checkin(date, m_sub_init_cash);
    }
}

DatetimeList MultiSystem::calcAdjustDates(const DatetimeList& dates, const string& mode,
                                          int adjust_cycle, bool delay_to_trading_day) {
    std::set<Datetime> result;
    if (dates.empty()) {
        return DatetimeList();
    }
    // Strictly restrict mode: only week/month/quarter/year are valid, the others (query/day/invalid
    // values) return empty
    string m = mode;
    to_lower(m);
    if (m != "week" && m != "month" && m != "quarter" && m != "year") {
        return DatetimeList();
    }
    const size_t total = dates.size();
    const int cycle = adjust_cycle > 0 ? adjust_cycle : 1;

    if (delay_to_trading_day) {
        // The postponement semantics: when the target day is not a trading day, it is postponed to
        // the first trading day within the current cycle (consistent with master Portfolio).
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
                adjust =
                  (date.dayOfWeek() ==
                   cycle);  // The cycle-th day of every week (0=Sunday, 1=Monday ... 6=Saturday)
            } else if (m == "month" || m == "quarter") {
                adjust = (date.day() == cycle);        // The cycle-th day of every month/quarter
            } else {                                   // year
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
    //   "kdata"    -- the date sequence of the input KData is used as the axis (the default,
    //   keeping the existing behavior) "calendar" -- the fixed date table (e.g. the market-wide
    //   trading calendar) injected by setDateAxis() is used as the axis;
    //                 the input KData degenerates into the query/ktype and the price query context
    //                 (runMoment/_closePhase only take its query)
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
    // master compatibility overload: equivalent to Portfolio::run(query), it uses the market
    // trading calendar as the driving axis
    auto& sm = StockManager::instance();

    // Consistent with master: when ktype is not the daily line, it is only allowed when adjust-mode
    // is query/day (the calendar axis is a daily line sequence)
    string mode = getAdjustMode();
    to_lower(mode);
    HKU_CHECK(mode == "query" || mode == "day" || query.kType() == KQuery::DAY,
              "The kType of query must be DAY when adjust-mode is not \"query\"! [{}]", name());

    // The driving axis: the explicitly injected fixed time axis takes precedence (respecting the
    // injection of the axis-mode=calendar users), otherwise the market trading calendar is used
    DatetimeList dates = (getAxisMode() == "calendar" && !m_date_axis.empty())
                           ? m_date_axis
                           : sm.getTradingCalendar(query);
    HKU_WARN_IF_RETURN(dates.empty(), void(), "No trading date in the query range! [{}]", name());

    // The context KData (only carrying the query/ktype and the price query context):
    //   its own instrument (explicitly set) -> the first (recursive) sub-system instrument -> the
    //   calendar benchmark index
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

void MultiSystem::_runAxis(const KData& kdata, const DatetimeList* axis, bool reset,
                           bool resetAll) {
    // master compatibility: materialize the sub-systems from the SE prototypes when needed (the
    // same call readyForRun makes, kept here so that this entry stays self-sufficient)
    _adoptSEProtos();
    HKU_WARN_IF_RETURN(m_sys_list.empty(), void(), "No subsystem specified!");
    m_kdata = kdata;
    // The KData cache is rebuilt with each run
    m_kdata_cache.clear();

    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    readyForRun();

    // Notify SE of the actually running system list (mapped with the prototypes), and start its
    // calculation
    if (m_se) {
        m_se->calculate(m_sys_list, m_kdata.getQuery());
    }

    // The account date filtering consistent with the single-security System::run: only the bars
    // after [the account initialization day, the account last trade day] are driven. Purpose: when
    // the live trading replays with the BrokerTM full alignment of the time axis daily, skip the
    // historical bars already executed on the real account,
    //       to avoid the duplicate orders polluting the real account. For a brand-new backtesting
    //       account (lastDatetime == initDatetime == the start of the time axis) this filtering is
    //       a no-op, it does not affect the portfolio backtesting traversing the whole axis.
    Datetime tm_init_datetime = m_tm->initDatetime();
    Datetime tm_last_datetime = m_tm->lastDatetime();
    if (KQuery::getKTypeInSeconds(m_kdata.getQuery().kType()) >= 86400) {
        tm_init_datetime = tm_init_datetime.startOfDay();
        tm_last_datetime = tm_last_datetime.startOfDay();
    }

    // adjust-mode internalization -- when it is not query/day, expand the rebalancing day table
    // on the "driving axis". The expansion only depends on the driving axis
    // itself, decoupled from the driving loop; the external setAdjustDates() injection takes
    // precedence.
    _expandAdjustDates(axis ? *axis : m_kdata.getDatetimeList());

    // master compatibility: build the rebalancing-day -> cycle-end mapping (the next rebalancing
    // day), used to drive the cycle-type signals of the sub-systems on the rebalancing day
    _buildCycleEnds(axis ? *axis : m_kdata.getDatetimeList());

    if (axis) {
        // Driven by the fixed time axis: the dates on the axis may not exist in the input KData
        // (the suspended/non-trading days do not constitute a gap)
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
                                                const FundsRecord& funds_before,
                                                const Datetime& datetime) const {
    TradeSuggestionList result;
    // The "before-trade" fund benchmark of the sub-system (to prevent division by zero): used to
    // calculate the three ratios of the suggestion (the complete semantic pass-through of the
    // design 8.2/8.3)
    double base_assets = funds_before.total_assets();
    double base_cash = funds_before.cash;
    std::map<Stock, double> net;     // The net quantity (positive=buy, negative=sell)
    std::map<Stock, price_t> price;  // The planned price (the traded price)

    // The aggregate layer does not support the short pass-through: ignore, to avoid the short
    // trades being executed as long-side trades
    static std::once_flag g_short_suggestion_warned;
    for (const auto& tr : trades) {
        if (tr.business == BUSINESS_INVALID) {
            continue;
        }
        if (tr.business == BUSINESS_BUY) {
            net[tr.stock] += tr.number;
        } else if (tr.business == BUSINESS_SELL) {
            net[tr.stock] -= tr.number;
        } else if (tr.business == BUSINESS_BUY_SHORT || tr.business == BUSINESS_SELL_SHORT) {
            std::call_once(g_short_suggestion_warned, [] {
                HKU_WARN(
                  "The aggregate system ignores the sub-system short trades (the short "
                  "pass-through "
                  "is not supported yet)!");
            });
            continue;
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
        s.plan_cash = s.number * s.plan_price * stock.unit();
        s.from = PART_SYSTEM;
        if (n > 0.0) {
            s.type = SuggestionType::BUY;
        } else {
            // The trade record carries the actual traded quantity, judge the full-position
            // clearance by whether the sub-system holding has been cleared
            double remain = sys->getTM() ? sys->getTM()->getPosition(datetime, stock).number : 0.0;
            s.type = remain <= 0.0 ? SuggestionType::CLEAR : SuggestionType::SELL;
        }
        // The three ratios: the sub-system before-trade funds are used as the denominator. The
        // parent MM maps them into the parent real assets by the ratios (mode A).
        if (base_assets > 0.0) {
            s.assets_ratio = s.plan_cash / base_assets;
            s.target_position_ratio =
              s.assets_ratio;  // Approximation: the target position ratio when a single trade
                               // builds the position from 0
        }
        if (base_cash > 0.0) {
            s.cash_ratio = s.plan_cash / base_cash;
        }
        result.push_back(s);
    }
    return result;
}

bool MultiSystem::_isAdjustDate(const Datetime& date) const {
    // The external rebalancing day table takes precedence: the rebalancing is only performed on the
    // dates hitting the table (used when PF maps the master adjust_mode / delay_to_trading_day)
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
        // SELL: sell the suggested quantity (in mode A it is rewritten to "-the parent current
        // position" = full close; in mode B the sub-instruction is passed through) CLEAR: exit the
        // whole position
        double num = s.number;
        if (s.type == SuggestionType::CLEAR || num >= MAX_DOUBLE) {
            num = MAX_DOUBLE;
        } else if (num < 0.0) {
            num = -num;
        }
        if (num <= 0.0) {
            continue;
        }
        // No quote (e.g. suspended): a 0-price sell would wipe the holding, skip it
        if (s.plan_price <= 0.0) {
            HKU_WARN("Skip the {} suggestion, {} has no quote on {}!",
                     s.type == SuggestionType::CLEAR ? "CLEAR" : "SELL", s.stock.market_code(),
                     date);
            continue;
        }
        TradeRecord tr = m_tm->sell(date, s.stock, s.plan_price, num, 0.0, 0.0, s.plan_price,
                                    PART_SYSTEM, "MultiSystem");
        if (!tr.isNull()) {
            out_trades.push_back(tr);
        }
    }

    // L3 cash feasibility: when the total BUY demand exceeds the available cash, scale it down
    // proportionally, to avoid the over-limit BUY orders being silently rejected by the TM
    double needed_cash = 0.0;
    for (const auto& s : suggestions) {
        if (s.type != SuggestionType::BUY || s.number <= 0.0 || s.plan_price <= 0.0) {
            continue;
        }
        double min_trade = s.stock.minTradeNumber();
        double qty = min_trade > 0.0 ? std::floor(s.number / min_trade) * min_trade : s.number;
        needed_cash += qty * s.plan_price * s.stock.unit();
    }
    double cash_scale = 1.0;
    if (needed_cash > 0.0) {
        double available = m_tm->getFunds(date, ktype).cash;
        if (needed_cash > available) {
            cash_scale = available / needed_cash;
            HKU_WARN(
              "The BUY suggestions need {:.2f} cash but only {:.2f} available, scale down "
              "by {:.2f}!",
              needed_cash, available, cash_scale);
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
        if (min_trade <= 0.0) {
            HKU_WARN("Invalid minTradeNumber {} of {}, skip the BUY suggestion!", min_trade,
                     s.stock.market_code());
            continue;
        }
        double qty = std::floor(s.number * cash_scale / min_trade) * min_trade;
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

    // The open stage: drive the sub-systems to fulfill the delayed requests, and collect the open
    // trades (used by the close stage to aggregate into the parent suggestions)
    MomentResult open_result = runMomentOnOpen(datetime);
    result.tradesOnOpen = open_result.tradesOnOpen;

    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    // The close stage: drive the sub-systems to generate signals, merge the "open+close" trades and
    // aggregate them into the suggestions, the parent orders uniformly
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
    // The parent handles the delisted instruments uniformly at the open stage: force selling the
    // parent holdings
    TradeRecordList delist_trades = _forceSellDelisted(datetime);
    for (auto& tr : delist_trades) {
        result.tradesOnOpen.push_back(tr);
        m_trade_list.push_back(tr);
    }

    // trade_on_close=false: the suggestions converted on the last rebalancing day are executed
    // uniformly at this open (re-priced by the open price of the day)
    if (!m_trade_on_close && !m_open_pending_suggestions.empty()) {
        for (auto& s : m_open_pending_suggestions) {
            price_t op = _getOpenPrice(datetime, s.stock);
            if (op > 0.0) {
                s.plan_price = op;
            }
        }
        KQuery::KType kt = m_kdata.getQuery().kType();
        TradeRecordList executed_open;
        _executeSuggestions(datetime, m_open_pending_suggestions, kt, executed_open);
        for (auto& tr : executed_open) {
            result.tradesOnOpen.push_back(tr);
            m_trade_list.push_back(tr);
        }
        m_open_pending_suggestions.clear();
    }

    // A new day: clear and rebuild the open trade buffer of every sub-system (used by the close
    // stage of this layer for the aggregation). Note: the open trades of the sub-systems happen on
    // their own (virtual) accounts, they are only cached into m_open_trades for the close merge of
    // this layer; they are [not counted] into the tradesOnOpen of the parent itself -- otherwise
    // when this MultiSystem is a sub-system of an upper aggregate, the upper layer would count
    // these grand-system raw open trades and the net trades already executed by this layer's close
    // together (nested double counting), causing the wrong direction/quantity of the parent
    // suggestion. The open trades of the parent's own account (e.g. the delisting forced
    // liquidation) have been merged into result.tradesOnOpen above, conforming to the MomentResult
    // contract.
    m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
    m_sub_funds_before.assign(m_sys_list.size(), FundsRecord{});
    m_open_trades_date = datetime;
    KQuery::KType ktype = m_kdata.getQuery().kType();
    const bool mode_c = (getMode() == "C");
    if (mode_c) {
        // The legacy open stage of the Portfolio without AF, part 1: the force-sell pool is swept
        // first. A sub-system whose holding reached zero leaves the pool; while the forced
        // liquidation is on, the still held ones are sold at the open.
        for (auto iter = m_force_sell_list.begin(); iter != m_force_sell_list.end();) {
            Stock stk = (*iter)->getStock();
            double num = stk.isNull() ? 0.0 : m_tm->getHoldNumber(datetime, stk);
            if (num <= 0.0) {
                iter = m_force_sell_list.erase(iter);
                continue;
            }
            if (m_sell_at_not_selected) {
                TradeRecord tr = (*iter)->sellForceOnOpen(datetime, num, PART_PORTFOLIO);
                if (!tr.isNull()) {
                    result.tradesOnOpen.push_back(tr);
                    m_trade_list.push_back(tr);
                }
            }
            ++iter;
        }
    }

    // The "before-trade" fund snapshot of every sub-system on that day (before the open drive),
    // used by the close stage _toSuggestions to calculate the three ratios. Mode C takes it for all
    // of them (also for the ones it does not drive), so that a sub-system admitted the very same
    // day still has a valid ratio denominator.
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        if (m_sys_list[i]->getTM()) {
            m_sub_funds_before[i] = m_sys_list[i]->getTM()->getFunds(datetime, ktype);
        }
    }

    // The sub-systems to drive at the open. Mode C drives the pools in their own order (the cash
    // competition order of the shared account) and never drives a sub-system that the SE has not
    // admitted, so an entry cannot happen behind the SE gate. @note the legacy engine skipped the
    // open drive on the rebalancing days altogether, which strands the delayed requests when
    // trade_on_close=false; here the open drive is kept on every day instead, which is neutral for
    // the golden case (trade_on_close=true produces no delayed request at all).
    SystemList drive_list;
    if (mode_c) {
        if (m_sell_at_not_selected) {
            drive_list.insert(drive_list.end(), m_force_sell_list.begin(), m_force_sell_list.end());
        }
        if (m_se) {
            drive_list.insert(drive_list.end(), m_running_order.begin(), m_running_order.end());
        } else {
            drive_list = m_sys_list;
        }
    } else {
        drive_list = m_sys_list;
    }

    for (auto& sys : drive_list) {
        size_t i = _subIndex(sys);
        if (i == Null<size_t>()) {
            continue;
        }
        MomentResult sub = sys->runMomentOnOpen(datetime);
        for (auto& tr : sub.tradesOnOpen) {
            m_open_trades[i].push_back(tr);
            if (mode_c) {
                // Mode C: those open trades happen ON the real account of the parent, so they are
                // surfaced by this layer as well; in mode A/B they stay shadow bookkeeping and are
                // deliberately not counted here (see the nested double counting note above).
                result.tradesOnOpen.push_back(tr);
                m_trade_list.push_back(tr);
            }
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

    // Prevent out-of-bounds + prevent cross-day residue: ensure the open buffer is aligned with the
    // sub-system quantity, and it is only reused when the buffer belongs to the current trading
    // day. Scenario: if only the close drive is registered during the live trading (runMomentOnOpen
    // is not called first on that day) or it has just gone through reset,
    //       m_open_trades may be empty (the [i] indexing below would crash out-of-bounds) or hold
    //       the open trades of the previous trading day (duplicate merging with the today close
    //       suggestions -> duplicate orders). The isNull() pre-short-circuit avoids calling
    //       startOfDay() on Null.
    if (m_open_trades.size() != m_sys_list.size() ||
        m_sub_funds_before.size() != m_sys_list.size() || m_open_trades_date.isNull() ||
        m_open_trades_date.startOfDay() != datetime.startOfDay()) {
        m_open_trades.assign(m_sys_list.size(), TradeRecordList{});
        m_sub_funds_before.assign(m_sys_list.size(), FundsRecord{});
        m_open_trades_date = datetime;
        // The before-trade snapshot is missing (e.g. only the close drive is registered during the
        // session, runMomentOnOpen is not called first): use the current funds of the sub-system as
        // the fallback, to ensure the ratio denominator is not empty
        KQuery::KType kt = m_kdata.getQuery().kType();
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            if (m_sys_list[i]->getTM()) {
                m_sub_funds_before[i] = m_sys_list[i]->getTM()->getFunds(datetime, kt);
            }
        }
    }

    bool is_adjust = _isAdjustDate(datetime);
    // Mode C (the shared account of the legacy Portfolio): the sub-system trades directly on the
    // real account of the parent (shared_tm), so this layer neither converts the suggestions nor
    // orders again. Its driven set is the running pool (plus the force-sell pool when the forced
    // liquidation is on), updated by the SE selection of the rebalancing day -- the dual pools of
    // the legacy engine are implemented in _closePhaseModeC.
    const bool mode_c = (getMode() == "C");
    TradeRecordList executed;  // The parent executed trades (incl. the immediate calibration sells)

    // The SE stock selection on the rebalancing day: only collect the suggestions of the selected
    // sub-systems; the unselected ones are liquidated by sell_at_not_selected. The SE filtering is
    // not enabled on the non-rebalancing days (every sub-system runs normally).
    std::set<System*> selected;
    SystemList se_selected;  // The same content in the order returned by the SE (mode C admits the
                             // pool in that order, as the legacy Portfolio did)
    std::unordered_map<System*, double> se_scores;  // The SE scores, used by AF_MultiFactor
                                                    // etc. to take the scores as the weights
    if (m_se && is_adjust) {
        SystemWeightList sws = m_se->getSelected(datetime);
        for (auto& sw : sws) {
            if (sw.sys) {
                selected.insert(sw.sys.get());
                se_selected.push_back(sw.sys);
                se_scores[sw.sys.get()] = sw.weight;
            }
        }
    }

    // Mode B (master SimplePortfolio compatibility): on the rebalancing day the quota must be
    // allocated BEFORE driving the sub-systems, so every selected sub-system trades with its exact
    // allocated quota (the master AF adjusted the funds first and ran the systems afterwards)
    bool quota_calibrated = false;
    AllocateFundsBase::Weights af_weights;
    std::unordered_map<System*, price_t> quota_map;
    if (is_adjust && getMode() == "B" && m_se && getAF()) {
        SubSystemContextList quota_ctxs;
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            SubSystemContext ctx;
            ctx.sys = sys;
            if (sys->getTM()) {
                ctx.funds = sys->getTM()->getFunds(datetime, ktype);
            }
            quota_ctxs.push_back(ctx);
        }
        if (!quota_ctxs.empty()) {
            af_weights = m_af->allocateQuota(datetime, m_tm, quota_ctxs, m_kdata.getQuery());
            for (auto& ctx : quota_ctxs) {
                quota_map[ctx.sys.get()] = ctx.quota;
            }
            quota_calibrated = true;
        }
    }

    // Mode B (master SimplePortfolio compatibility): the calibration runs in the master order --
    // recycle the cash -> liquidate the unselected -> reduce the over quota -> inject the gap, so
    // the freed cash is already in the parent pool when the following injection runs
    if (quota_calibrated) {
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                // The unselected: the parent liquidates its holding immediately (the master AF
                // force sold inside the sub account and recorded it in the total account)
                if (m_sell_at_not_selected && !sys->getStock().isNull() &&
                    m_tm->have(sys->getStock())) {
                    price_t price = _getClosePrice(datetime, sys->getStock());
                    if (price > 0.0) {
                        TradeRecord tr =
                          m_tm->sell(datetime, sys->getStock(), price,
                                     m_tm->getPosition(datetime, sys->getStock()).number, 0.0, 0.0,
                                     price, PART_PORTFOLIO, "MultiSystem");
                        if (!tr.isNull()) {
                            executed.push_back(tr);
                        }
                    }
                }
                _clearSubSystem(sys, datetime, ktype);
            } else {
                // The selected: recycle the sub cash first (bookkeeping)
                TMPtr sub_tm = sys->getTM();
                if (sub_tm) {
                    FundsRecord funds = sub_tm->getFunds(datetime, ktype);
                    if (funds.cash > 0.0) {
                        sub_tm->checkout(datetime, funds.cash);
                    }
                }
            }
        }
        // The reduction of all the over quota sub-systems runs before the injection (the master
        // AF order), so the freed cash is available to the injection phase
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            auto qit = quota_map.find(sys.get());
            if (qit != quota_map.end()) {
                _reduceSubSystemToQuota(sys, datetime, qit->second, ktype, executed);
            }
        }
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            auto qit = quota_map.find(sys.get());
            if (qit != quota_map.end()) {
                _injectSubSystemGap(sys, datetime, qit->second, ktype);
            }
        }
    }

    if (mode_c) {
        _closePhaseModeC(datetime, is_adjust, se_selected, suggestions, executed);
        m_close_day_index++;
        // Mode C: nothing is accumulated as pending and the parent does not order again (the trades
        // have already landed on the shared real account). The translated suggestions are still
        // kept in m_last_suggestions, so that this system can serve as a sub-system of an upper
        // mode A/B aggregate (the nesting translation of the design 9.3); the ratio denominator is
        // the shared account funds taken from m_sub_funds_before.
        m_last_suggestions = suggestions;
        _finishClosePhase(datetime, suggestions, executed, is_adjust);
        return executed;
    }

    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr sys = m_sys_list[i];
        const bool is_selected = (selected.count(sys.get()) > 0);
        if (m_se && is_adjust && !is_selected) {
            // The unselected sub-system: it has been liquidated (executed immediately) when the
            // quota was calibrated; in the legacy flow (no SE / mode A) the liquidation suggestion
            // is generated here
            if (!quota_calibrated && m_sell_at_not_selected && !sys->getStock().isNull() &&
                m_tm->have(sys->getStock())) {
                TradeSuggestion s;
                s.stock = sys->getStock();
                s.sys = sys;
                s.type = SuggestionType::CLEAR;
                s.plan_price = _getClosePrice(datetime, s.stock);
                s.number = m_tm->getPosition(datetime, s.stock).number;
                suggestions.push_back(s);
            }
            // Mode B (master compatibility): clear the sub-system shadow account as well (the
            // master AF liquidated the sub account and recycled all its funds)
            if (getMode() == "B" && !quota_calibrated) {
                _clearSubSystem(sys, datetime, ktype);
            }
            continue;
        }

        // Mode A (signal aggregation): reset the shadow signal cash on the rebalancing day (once
        // a day), so every sub-system can submit a fresh position intent -- the shadow is a pure
        // signal source and would run out of cash after its first position otherwise, leaving the
        // parent cash idle and the re-entered stocks impossible to buy back
        // @note This branch is mode A only (it used to be written as "!= B", which was fine
        //       while mode C did not exist): mode C has no shadow account (its sub_tm IS the real
        //       account of the parent), so clearing/injecting the signal cash there would wipe the
        //       parent positions and mint phantom cash.
        if (is_adjust && getMode() == "A" &&
            (m_signal_reset_date.isNull() || datetime > m_signal_reset_date)) {
            bool reset_this = m_se ? (selected.count(sys.get()) > 0) : true;
            if (reset_this) {
                _resetSubSystemSignalCash(sys, datetime, ktype);
                if (i < m_sub_funds_before.size() && sys->getTM()) {
                    m_sub_funds_before[i] = sys->getTM()->getFunds(datetime, ktype);
                }
                m_signal_reset_date = datetime;
            }
        }

        // master compatibility: on the rebalancing day, drive the cycle-type signals (e.g.
        // SG_Cycle, which emits its buy signal only at the start of a cycle) before running the
        // selected sub-system, as SimplePortfolio did for every running system
        if (is_adjust) {
            auto sg = sys->getSG();
            if (sg) {
                sg->startCycle(datetime, _getNextCycleEnd(datetime));
            }
        }
        MomentResult sub = sys->runMomentOnClose(datetime);
        // The sub-system decision may be reflected in the open trade (the delayed buy) or the close
        // trade (the immediate buy/sell), it is merged and translated into the parent suggestion
        // (mode A/B share this glue). The open buffer is consumed once merged, to avoid the
        // duplicate merging when the close is driven multiple times in the same day.
        TradeRecordList sub_trades = m_open_trades[i];
        m_open_trades[i].clear();
        sub_trades.insert(sub_trades.end(), sub.tradesOnClose.begin(), sub.tradesOnClose.end());
        TradeSuggestionList subsug =
          _toSuggestions(sys, sub_trades, m_sub_funds_before[i], datetime);
        for (auto& s : subsug) {
            suggestions.push_back(s);
        }
        SubSystemContext ctx;
        ctx.sys = sys;
        ctx.funds = sys->getTM()->getFunds(datetime, ktype);
        // Backfill the SE score (0 on the non-rebalancing days / for the unselected ones), used
        // by AF_MultiFactor etc. to take the scores as the weights
        auto score_it = se_scores.find(sys.get());
        if (score_it != se_scores.end()) {
            ctx.score = score_it->second;
        }
        contexts.push_back(ctx);
    }

    m_close_day_index++;

    if (!is_adjust) {
        // Non-rebalancing day: accumulate the suggestions to the next rebalancing day
        m_pending_suggestions.insert(m_pending_suggestions.end(), suggestions.begin(),
                                     suggestions.end());
        m_last_suggestions = suggestions;
        return TradeRecordList();
    }

    // Rebalancing day: merge the pending suggestions; the unselected sub-systems' ones are
    // dropped (their parent holdings have been cleared as the unselected above)
    if (m_se) {
        for (auto& s : m_pending_suggestions) {
            if (s.sys && selected.count(s.sys.get()) > 0) {
                suggestions.push_back(s);
            }
        }
    } else {
        suggestions.insert(suggestions.end(), m_pending_suggestions.begin(),
                           m_pending_suggestions.end());
    }
    m_pending_suggestions.clear();

    if (getMode() == "B") {
        if (quota_calibrated) {
            // The quota has been calibrated into the sub-system shadow accounts before driving
            // (the master order: adjust the funds first, run the systems afterwards). Here only
            // the L2 pass-through and the L3 risk control remain, then the parent orders uniformly.
            getAF()->allocateTargets(datetime, m_tm, suggestions, af_weights, m_kdata.getQuery());
            if (!suggestions.empty()) {
                if (m_trade_on_close) {
                    _executeSuggestions(datetime, suggestions, ktype, executed);
                } else {
                    // trade_on_close=false: convert first, execute uniformly at the next open
                    m_open_pending_suggestions = suggestions;
                }
            }
        } else {
            // No SE (or no quota pre-calculation): keep the suggestion-driven legacy flow, L1 runs
            // here and the next-period quota is written back on the rebalancing day (lagging one
            // period behind, quota penetration).
            getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
            // The selected but zero-quota sub-systems are handled as the unselected ones
            if (m_sell_at_not_selected) {
                for (auto& ctx : contexts) {
                    if (ctx.quota <= 0.0 && !ctx.sys->getStock().isNull() &&
                        m_tm->have(ctx.sys->getStock())) {
                        TradeSuggestion s;
                        s.stock = ctx.sys->getStock();
                        s.sys = ctx.sys;
                        s.type = SuggestionType::CLEAR;
                        s.plan_price = _getClosePrice(datetime, s.stock);
                        s.number = m_tm->getPosition(datetime, s.stock).number;
                        suggestions.push_back(s);
                    }
                }
            }
            if (!suggestions.empty()) {
                if (m_trade_on_close) {
                    _executeSuggestions(datetime, suggestions, ktype, executed);
                } else {
                    // trade_on_close=false: convert first, execute uniformly at the next open
                    m_open_pending_suggestions = suggestions;
                }
            }
            for (auto& ctx : contexts) {
                if (ctx.quota > 0.0) {
                    setSubSystemQuota(ctx.sys, datetime, ctx.quota);
                }
            }
        }
    } else if (!suggestions.empty()) {
        // Mode A: after the L2 conversion of AF, the parent orders uniformly
        getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery());
        if (m_trade_on_close) {
            _executeSuggestions(datetime, suggestions, ktype, executed);
        } else {
            // trade_on_close=false: convert first, execute uniformly at the next open
            m_open_pending_suggestions = suggestions;
        }
    }

    m_last_suggestions = suggestions;
    _finishClosePhase(datetime, suggestions, executed, is_adjust);
    return executed;
}

void MultiSystem::_finishClosePhase(const Datetime& datetime,
                                    const TradeSuggestionList& suggestions,
                                    const TradeRecordList& executed, bool is_adjust) {
    KQuery::KType ktype = m_kdata.getQuery().kType();

    // The rebalancing turnover rate: the turnover amount / the total assets before rebalancing
    // (only recorded on the rebalancing days with actual rebalancing trades)
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
            const char* typ = s.type == SuggestionType::BUY
                                ? "BUY"
                                : (s.type == SuggestionType::CLEAR ? "CLEAR" : "SELL");
            HKU_INFO("[{}]   sug {} {} num={:.2f} price={:.3f}", getPath(), s.stock.market_code(),
                     typ, s.number, s.plan_price);
        }
    }
}

void MultiSystem::_adoptSEProtos() {
    if (!m_sys_list.empty() || !m_se) {
        return;
    }
    for (const auto& proto : m_se->getProtoSystemList()) {
        add(proto);
    }
}

size_t MultiSystem::_subIndex(const SystemPtr& sys) const {
    auto iter = m_sub_index.find(sys.get());
    return iter != m_sub_index.end() ? iter->second : Null<size_t>();
}

bool MultiSystem::_addToRunning(const SystemPtr& sys) {
    if (m_running_set.count(sys.get()) > 0) {
        return false;
    }
    m_running_set.insert(sys.get());
    m_running_order.push_back(sys);
    return true;
}

void MultiSystem::_removeFromRunning(const SystemPtr& sys) {
    m_running_set.erase(sys.get());
    for (auto it = m_running_order.begin(); it != m_running_order.end(); ++it) {
        if (it->get() == sys.get()) {
            m_running_order.erase(it);
            break;
        }
    }
}

void MultiSystem::_removeStockFromPoolsRecursive(const Stock& stock) {
    for (auto& sys : m_sys_list) {
        if (!sys) {
            continue;
        }
        if (sys->getStock() == stock) {
            _removeFromRunning(sys);
            for (auto it = m_force_sell_list.begin(); it != m_force_sell_list.end(); ++it) {
                if (it->get() == sys.get()) {
                    m_force_sell_list.erase(it);
                    break;
                }
            }
        } else if (sys->isComposite()) {
            // A nested aggregate keeps its own pools; the delisting must be reflected there too
            if (auto* inner = dynamic_cast<MultiSystem*>(sys.get())) {
                inner->_removeStockFromPoolsRecursive(stock);
            }
        }
    }
}

void MultiSystem::_runModeCClose(const SystemPtr& sys, const Datetime& datetime,
                                 TradeSuggestionList& suggestions, TradeRecordList& executed) {
    size_t i = _subIndex(sys);
    if (i == Null<size_t>()) {
        return;
    }
    MomentResult sub = sys->runMomentOnClose(datetime);
    // Mode C: the trades of the sub-system ARE the trades of the real account of the parent, they
    // are surfaced as they are (the parent must not order again). The open trades have already been
    // surfaced by the open stage, so only the close ones are pushed here, no double counting.
    for (auto& tr : sub.tradesOnClose) {
        executed.push_back(tr);
    }
    // The translated suggestions are still produced (the nesting case: this system may be a
    // sub-system of an upper mode A/B aggregate, which reads them via toSuggestions()); the open
    // buffer is consumed once merged, so a repeated close drive of the same day cannot duplicate
    // it.
    TradeRecordList sub_trades = m_open_trades[i];
    m_open_trades[i].clear();
    sub_trades.insert(sub_trades.end(), sub.tradesOnClose.begin(), sub.tradesOnClose.end());
    TradeSuggestionList subsug = _toSuggestions(sys, sub_trades, m_sub_funds_before[i], datetime);
    for (auto& s : subsug) {
        suggestions.push_back(s);
    }
}

void MultiSystem::_closePhaseModeC(const Datetime& datetime, bool is_adjust,
                                   const SystemList& se_selected, TradeSuggestionList& suggestions,
                                   TradeRecordList& executed) {
    // Without SE every sub-system is driven every day (equivalent to an "all selected" SE): the
    // legacy Portfolio always had an SE, this keeps the mode well-defined anyway.
    if (!m_se) {
        for (auto& sys : m_sys_list) {
            _runModeCClose(sys, datetime, suggestions, executed);
        }
        return;
    }

    if (!is_adjust) {
        // Non-rebalancing day (legacy): the pools are not updated; the force-sell pool is driven
        // only when the forced liquidation is on, then the running pool is driven.
        if (m_sell_at_not_selected) {
            for (auto& sys : m_force_sell_list) {
                _runModeCClose(sys, datetime, suggestions, executed);
            }
        }
        for (auto& sys : m_running_order) {
            _runModeCClose(sys, datetime, suggestions, executed);
        }
        return;
    }

    // Rebalancing day: will_remove = running - selected, whatever the holding state (the legacy
    // rule; an "until it is closed" rule would keep watching the position and would not reproduce
    // the legacy results)
    std::set<System*> keep;
    for (auto& sys : se_selected) {
        keep.insert(sys.get());
    }
    SystemList will_remove;
    for (auto& sys : m_running_order) {
        if (keep.count(sys.get()) == 0) {
            will_remove.push_back(sys);
        }
    }
    for (auto& sys : will_remove) {
        _removeFromRunning(sys);
    }

    // The admitted ones enter the pool (in the SE order) and ONLY the new ones get startCycle, as
    // the legacy Portfolio did
    for (auto& sys : se_selected) {
        if (_addToRunning(sys)) {
            // Being admitted again also takes the sub-system out of the force-sell pool: the two
            // pools stay mutually exclusive (the legacy engine left the stale entry behind, which
            // then made the open stage force-sell a system the SE had just re-admitted and drove it
            // twice in the same open). This is neutral for the golden case, where the forced
            // liquidation is off and the force-sell pool is never driven.
            for (auto it = m_force_sell_list.begin(); it != m_force_sell_list.end();) {
                if (it->get() == sys.get()) {
                    it = m_force_sell_list.erase(it);
                } else {
                    ++it;
                }
            }
            auto sg = sys->getSG();
            if (sg) {
                sg->startCycle(datetime, _getNextCycleEnd(datetime));
            }
        }
    }

    // Drive the running pool in the insertion order: with one shared real account this is exactly
    // the cash competition order of the legacy engine (an empty sub-system opens a fresh position,
    // a held one may add to it by its own MM, C-O2=B)
    for (auto& sys : m_running_order) {
        _runModeCClose(sys, datetime, suggestions, executed);
    }

    // Then the removed ones: forced liquidation when asked to, otherwise ONE last drive (so that a
    // sell signal of that very day is still realized); whatever still holds afterwards goes to the
    // force-sell pool, where it stops being watched when the forced liquidation is off.
    for (auto& sys : will_remove) {
        Stock stk = sys->getStock();
        double num = stk.isNull() ? 0.0 : m_tm->getHoldNumber(datetime, stk);
        if (num <= 0.0) {
            continue;
        }
        if (m_sell_at_not_selected) {
            TradeRecord tr = sys->sellForceOnClose(datetime, num, PART_PORTFOLIO);
            if (!tr.isNull()) {
                executed.push_back(tr);
            }
        } else {
            _runModeCClose(sys, datetime, suggestions, executed);
        }
        if (!stk.isNull() && m_tm->have(stk)) {
            bool already = false;
            for (auto& held : m_force_sell_list) {
                if (held.get() == sys.get()) {
                    already = true;
                    break;
                }
            }
            if (!already) {
                m_force_sell_list.emplace_back(sys);
            }
        }
    }
}

KData MultiSystem::_getStockKData(const Stock& stock) const {
    // The KData of the instrument within the run query is cached (rebuilding it per bar is an
    // obvious overhead in the large-combination minute-line scenario); the cache is cleared with
    // each run (_runAxis) and reset
    auto iter = m_kdata_cache.find(stock.market_code());
    if (iter != m_kdata_cache.end()) {
        return iter->second;
    }
    KData kdata = stock.getKData(m_kdata.getQuery());
    m_kdata_cache.emplace(stock.market_code(), kdata);
    return kdata;
}

price_t MultiSystem::_getClosePrice(const Datetime& date, const Stock& stock) const {
    if (stock.isNull()) {
        return 0.0;
    }
    KData kdata = _getStockKData(stock);
    size_t pos = kdata.getPos(date);
    return pos == Null<size_t>() ? 0.0 : kdata.getKRecord(pos).closePrice;
}

price_t MultiSystem::_getOpenPrice(const Datetime& date, const Stock& stock) const {
    if (stock.isNull()) {
        return 0.0;
    }
    KData kdata = _getStockKData(stock);
    size_t pos = kdata.getPos(date);
    return pos == Null<size_t>() ? 0.0 : kdata.getKRecord(pos).openPrice;
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
        // Judge the delisting by the last bar time of the instrument itself, rather than the
        // last bar within the query window (the axis may be longer than the window)
        Datetime last_dt = pos.stock.lastDatetime();
        if (last_dt == Null<Datetime>() || last_dt >= date) {
            continue;
        }
        // The sub-system trading this delisted instrument leaves the mode C pools at once, no
        // matter whether the liquidation below succeeds: it can never trade again, and the pools
        // are the "watched" sets (the design 9.3 delisting rule)
        _removeStockFromPoolsRecursive(pos.stock);
        // The last trading day of the instrument has passed (delisting): force liquidation at the
        // close price of the last trading day within the query window
        KData kdata = _getStockKData(pos.stock);
        if (kdata.empty()) {
            continue;
        }
        price_t price = kdata.getKRecord(kdata.size() - 1).closePrice;
        TradeRecord tr =
          m_tm->sell(date, pos.stock, price, MAX_DOUBLE, 0.0, 0.0, price, PART_SYSTEM, "DELIST");
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
    //   - Quota decrease: withdraw the cash (checkout the difference; when the cash is insufficient
    //   the sub-system reduces the position itself, a warning is recorded here)
    // The aggregate sub-system (nested) also triggers its internal allocation by adjusting the
    // total assets of its virtual account (quota penetration).
    KQuery::KType ktype = m_kdata.getQuery().kType();
    if (ktype.empty()) {
        ktype = KQuery::DAY;
    }
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t diff = quota - funds.total_assets();
    if (diff > 0.0) {
        sub_tm->checkin(date, diff);
    } else if (diff < 0.0) {
        // The quota reduction needs free cash; when the sub-system is fully invested it cannot
        // return the cash itself (it would have to reduce the position on its own). Keep the
        // current quota (lags one period) and log at info level to avoid flooding the log with the
        // inner TradeManager checkout errors on every rebalancing day.
        if (funds.cash < -diff) {
            HKU_INFO(
              "Quota reduction {:.2f} exceeds sub cash({:.2f}), subsystem {} must reduce "
              "position itself, quota remains!",
              -diff, funds.cash, sub_sys->name());
            return;
        }
        sub_tm->checkout(date, -diff);
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
