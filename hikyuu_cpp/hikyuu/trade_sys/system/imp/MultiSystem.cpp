/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  Recursive combination refactoring: the aggregate trading system (portfolio backtesting)
 *  Three modes (A/B/C) + arbitrary nesting + AF L1/L2/L3 + rebalancing cycle + hierarchy path.
 */

#include "MultiSystem.h"

#include <algorithm>
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

MultiSystem::MultiSystem() : System() {
    _initAxisParam();
    // The default MM of the aggregate parent system is mode A (signal aggregation, the base
    // allocate implements the equal weight allocation)
    if (!getMM()) {
        setMM(MM_Nothing());
    }
}

MultiSystem::MultiSystem(const string& name) : System(name) {
    _initAxisParam();
    if (!getMM()) {
        setMM(MM_Nothing());
    }
}

MultiSystem::MultiSystem(const SystemPtr& sys) : m_sys_list(sys ? SystemList{sys} : SystemList{}) {
    _initAxisParam();
    if (!getMM()) {
        setMM(MM_Nothing());
    }
}

MultiSystem::MultiSystem(const SystemList& sys_list, const string& name)
: System(name), m_sys_list(sys_list) {
    _initAxisParam();
    if (!getMM()) {
        setMM(MM_Nothing());
    }
}

void MultiSystem::_initAxisParam() {
    setParam<string>("axis_mode", "kdata");
    // Aligned with the master PF adjust_mode / delay_to_trading_day
    setParam<string>("adjust_mode", "query");
    setParam<bool>("delay_to_trading_day", true);
    // Snapshotted into the run-time switches at readyForRun
    setParam<int>("adjust_cycle", 1);
    setParam<bool>("trade_on_close", true);
    setParam<bool>("sell_at_not_selected", false);
    setParam<bool>("auto_checkin", false);
}

void MultiSystem::_refreshRunConfig() {
    int cycle = getAdjustCycle();
    if (cycle <= 0) {
        HKU_WARN("Invalid adjust_cycle {} (<=0), treated as 1! [{}]", cycle, name());
        cycle = 1;
        m_params.set<int>("adjust_cycle", cycle);
    }
    string mode = getAdjustMode();
    string fixed = mode;
    to_lower(fixed);
    if (fixed != mode) {
        m_params.set<string>("adjust_mode", fixed);
    }
    const bool delay = getDelayToTradingDay();
    if (cycle != m_adjust_cycle || fixed != m_adjust_mode || delay != m_delay_to_trading_day) {
        m_adjust_cycle = cycle;
        m_adjust_mode = fixed;
        m_delay_to_trading_day = delay;
        _clearCycleDates();
        m_adjust_cycle_warned = false;
    }
    m_trade_on_close = getTradeOnClose();
    m_sell_at_not_selected = getSellAtNotSelected();
    m_auto_checkin = getAutoCheckin();
    if (!m_adjust_cycle_warned) {
        m_adjust_cycle_warned = true;
        _warnAdjustCycleOutOfRange();
    }
}

void MultiSystem::_checkParam(const string& name) const {
    // Only the fully illegal values are rejected here; the clamping/normalization and the
    // derived state reset belong to the run-time preparation (_refreshRunConfig)
    if (name == "adjust_cycle") {
        HKU_ASSERT(getParam<int>("adjust_cycle") >= 0);
    } else if (name == "adjust_mode") {
        string mode = getParam<string>("adjust_mode");
        to_lower(mode);
        HKU_CHECK(mode == "query" || mode == "day" || mode == "week" || mode == "month" ||
                    mode == "quarter" || mode == "year",
                  "Invalid adjust_mode: {}! only query/day/week/month/quarter/year supported",
                  getParam<string>("adjust_mode"));
    } else if (name == "axis_mode") {
        string mode = getParam<string>("axis_mode");
        HKU_CHECK(mode == "kdata" || mode == "calendar",
                  "Invalid axis_mode: {}! only \"kdata\" / \"calendar\" supported", mode);
    }
}

void MultiSystem::setMode(const string& mode) {
    string m = mode;
    to_upper(m);
    if (m != "A" && m != "B" && m != "C") {
        HKU_WARN("Invalid mode: {}, only \"A\"/\"B\"/\"C\" supported, fallback to \"A\"! [{}]",
                 mode, name());
        m = "A";
    }
    if (m != m_mode) {
        // The account form of the sub-systems follows the mode (a shadow account in A/B, the
        // shared real account in C) and so does the mode C pool: a mode change must recreate them
        // on the next readyForRun, otherwise the sub-systems would keep the account form of the
        // previous mode (e.g. trading directly on the real account after switching C -> A)
        m_shadow_sys.clear();
        m_running_set.clear();
        m_running_order.clear();
        m_force_sell_list.clear();
        _clearCycleDates();
    }
    m_mode = m;
}

bool MultiSystem::_subtreeHasNetShort(const SystemPtr& sys) {
    if (!sys) {
        return false;
    }
    // The node's own book (a nested aggregate holds on its own shadow account) ...
    TMPtr tm = sys->getTM();
    if (tm) {
        for (const auto& pos : tm->getShortPositionList()) {
            if (!pos.stock.isNull() && pos.number > 0.0) {
                return true;
            }
        }
    }
    // ... plus the recursive leaves: a nested aggregate has an empty getStock(), so a direct
    // getShortPosition(stock) check would never fire on it
    if (auto* ms = dynamic_cast<MultiSystem*>(sys.get())) {
        for (auto& s : ms->m_sys_list) {
            if (_subtreeHasNetShort(s)) {
                return true;
            }
        }
    }
    return false;
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

bool MultiSystem::hasPythonPart() const {
    return System::hasPythonPart() || (m_se && m_se->hasPythonPart()) ||
           (m_af && m_af->hasPythonPart());
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
    // MultiSystem overrides readyForRun without delegating to System::readyForRun, so the two
    // margin-trading switches have to be applied to the parent TM here
    m_tm->setParam<bool>("support_borrow_cash", getParam<bool>("support_borrow_cash"));
    m_tm->setParam<bool>("support_borrow_stock", getParam<bool>("support_borrow_stock"));
    // The SE-provided prototypes are materialized here as well, so that an aggregate configured
    // only with an SE can be prepared for running (the legacy Portfolio usage and the live trading
    // entry, which calls readyForRun before the first run)
    _adoptSEProtos();
    HKU_CHECK(!m_sys_list.empty(), "No subsystem specified! {}", name());

    _refreshRunConfig();

    // The rebalancing days are the run-level input of the cycle signals: push the days declared by
    // _runAxis down BEFORE the sub-system trading objects are (re)calculated. The entries without a
    // precomputed axis (runMoment* / intraday live / nested) declare each day at the close stage
    _injectCycleDates();

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
            // Restore the delay mapping of the legacy Portfolio: when the rebalancing is executed
            // at the close the orders of the sub-systems must not be delayed, otherwise the delayed
            // requests get dropped by the SE buy gate on the next rebalancing day (the reason why
            // the refactored engine produced no trade at all for the existing strategies).
            // Only a changed value is written: setParam always notifies and would invalidate the
            // component cache on every live replay day
            const bool delay = !m_trade_on_close;
            if (!sys->tryGetParam<bool>("shared_tm", false)) {
                sys->setParam<bool>("shared_tm", true);
            }
            if (sys->tryGetParam<bool>("buy_delay", false) != delay) {
                sys->setParam<bool>("buy_delay", delay);
            }
            if (sys->tryGetParam<bool>("sell_delay", false) != delay) {
                sys->setParam<bool>("sell_delay", delay);
            }
        } else {
            // The shadow account is created only once: re-creating it would wipe the sub-account
            // state of the reset=false live replay (the pending delayed requests, the mode B quota)
            // Mode B (master SimplePortfolio compatibility): the sub-account starts from zero and
            // is injected with the exact quota on every rebalancing day (the cost function follows
            // the parent account, so the shadow book keeps in step with the parent trades). The
            // quota cash is virtual: the injections never take the money out of the parent TM.
            // Mode A (signal aggregation): the fixed shadow cash (pure signal source).
            if (!m_shadow_sys.count(sys.get())) {
                price_t init_cash = (getMode() == "B") ? 0.0 : m_sub_init_cash;
                TMPtr sub_tm = crtTM(m_tm->initDatetime(), init_cash, m_tm->costFunc(), "TM_SUB");
                sys->setTM(sub_tm);
                m_shadow_sys.insert(sys.get());
            }
            if (sys->tryGetParam<bool>("shared_tm", false)) {
                sys->setParam<bool>("shared_tm", false);
            }
        }
        if (sys->getAggregateOwner() && sys->getAggregateOwner() != this &&
            !m_multi_parent_warned) {
            m_multi_parent_warned = true;
            HKU_WARN(
              "Sub-system {} is already driven by another aggregate; its account is "
              "re-parented to {} and the other aggregate will keep driving it too!",
              sys->name(), name());
        }
        sys->setAggregateOwner(this);
        // The hierarchy path is written recursively
        sys->setPath(m_path + "/" + sys->name());
        if (sys->isComposite()) {
            // A nested aggregate has no instrument of its own and its own run() is never called on
            // that path (the parent drives it bar by bar), so it inherits the driving KData of the
            // parent: otherwise its price queries and its own SE would be left with no time axis at
            // all and the nested layer could never trade
            auto inner = std::dynamic_pointer_cast<MultiSystem>(sys);
            if (inner && !m_kdata.empty() && inner->m_kdata != m_kdata) {
                inner->m_kdata = m_kdata;
                inner->m_kdata_cache.clear();
                inner->m_kdata_cache_mark = Datetime();
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
    m_open_pending_suggestions.clear();
    m_open_pending_date = Datetime();
    m_open_pending_assets = 0.0;
    m_open_pending_retry = false;
    m_open_pending_adjust = false;
    m_pending_calibration.clear();
    m_pending_calibration_date = Datetime();
    m_pending_calibration_assets = 0.0;
    m_open_trades_date = Datetime();
    m_kdata_cache.clear();
    m_kdata_cache_mark = Datetime();
    // The one-shot notices must become visible again after a reset: the risks they report
    // (truncated injection, dropped pending batch, skipped net-short calibration) still apply
    m_open_pending_warned = false;
    m_inject_truncated_warned = false;
    m_short_reduce_warned = false;
    m_short_inject_warned = false;
    m_no_se_cycle_warned = false;
    // mode C: the running pool and the force-sell pool are runtime states, dropped on reset
    m_running_set.clear();
    m_running_order.clear();
    m_force_sell_list.clear();
    // The rebalancing cycle restarts with a fresh run; kept across the live reset=false replays
    m_close_day_index = 0;
    m_last_close_date = Datetime();
    m_last_adjust_period = Datetime();
    m_last_sampled_date = Datetime();
    m_external_quota = 0.0;
    m_external_quota_set = false;
    m_external_delegate_warned = false;
    m_external_quota_capped_warned = false;
    m_multi_parent_warned = false;
    m_not_ready_warned = false;
    m_pending_calibration_retry = false;
    m_cycle_dates.clear();
    if (m_se) {
        // Only the cached selection is dropped: a full SE reset would also drop the
        // real-to-prototype bindings (the aggregate cannot rebuild them)
        m_se->invalidateCache();
    }
}

void MultiSystem::_forceResetAll() {
    // The instrument of a sub-system is the topology handed over by the parent / the SE, not the
    // running state; the full reset below wipes it, so snapshot it first and restore it after,
    // otherwise a resetAll run silently degrades into "no trades"
    std::vector<Stock> sub_stocks;
    sub_stocks.reserve(m_sys_list.size());
    for (auto& sys : m_sys_list) {
        sub_stocks.push_back(sys->getStock());
    }

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
    m_open_pending_suggestions.clear();
    m_open_pending_date = Datetime();
    m_open_pending_assets = 0.0;
    m_open_pending_retry = false;
    m_open_pending_adjust = false;
    m_pending_calibration.clear();
    m_pending_calibration_date = Datetime();
    m_pending_calibration_assets = 0.0;
    m_open_trades_date = Datetime();
    m_kdata_cache.clear();
    m_kdata_cache_mark = Datetime();
    // The one-shot notices must become visible again after a full reset
    m_open_pending_warned = false;
    m_inject_truncated_warned = false;
    m_short_reduce_warned = false;
    m_short_inject_warned = false;
    m_no_se_cycle_warned = false;
    // mode C: the running pool and the force-sell pool are runtime states, dropped on reset
    m_running_set.clear();
    m_running_order.clear();
    m_force_sell_list.clear();
    // The rebalancing cycle restarts with a fresh run; kept across the live reset=false replays
    m_close_day_index = 0;
    m_last_close_date = Datetime();
    m_last_adjust_period = Datetime();
    m_last_sampled_date = Datetime();
    m_external_quota = 0.0;
    m_external_quota_set = false;
    m_external_delegate_warned = false;
    m_external_quota_capped_warned = false;
    m_multi_parent_warned = false;
    m_not_ready_warned = false;
    m_pending_calibration_retry = false;
    m_cycle_dates.clear();
    if (m_se) {
        // Only the cached selection is dropped: a full SE reset would also drop the
        // real-to-prototype bindings (the aggregate cannot rebuild them)
        m_se->invalidateCache();
    }

    // Restore the instruments and drop the stale trading objects: readyForRun rebuilds the TO
    // from the restored instrument with the query of the new run
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        if (sub_stocks[i].isNull()) {
            continue;
        }
        m_sys_list[i]->setTO(KData());
        m_sys_list[i]->setStock(sub_stocks[i]);
    }
}

SystemPtr MultiSystem::_clone() {
    auto ret = make_shared<MultiSystem>(name());

    // The prototypes adopted from the SE are the running systems themselves. The clone re-adopts
    // the prototypes of the cloned SE (the same instances, keeping the identity real==proto
    // binding), cloning the running list separately would create a second disjoint instance set
    // and the logical selector would silently select nothing on the clone
    std::unordered_map<System*, SYSPtr> cloned_protos;
    if (m_se) {
        ret->m_se = m_se->clone();
        const auto& old_protos = m_se->getProtoSystemList();
        const auto& new_protos = ret->m_se->getProtoSystemList();
        if (old_protos.size() == new_protos.size()) {
            for (size_t i = 0, total = old_protos.size(); i < total; i++) {
                cloned_protos[old_protos[i].get()] = new_protos[i];
            }
        } else {
            HKU_WARN("The prototype count of the cloned SE is inconsistent: {} != {}",
                     old_protos.size(), new_protos.size());
        }
    }

    for (auto& sys : m_sys_list) {
        auto iter = cloned_protos.find(sys.get());
        SystemPtr new_sys;
        if (iter != cloned_protos.end()) {
            new_sys = iter->second;
            ret->m_se->bindRealToProto(new_sys, new_sys);
        } else {
            new_sys = sys->clone();
        }
        ret->add(new_sys);
        if (m_shadow_sys.count(sys.get())) {
            ret->m_shadow_sys.insert(new_sys.get());
        }
    }

    ret->m_sub_init_cash = m_sub_init_cash;
    ret->m_mode = m_mode;            // The running mode is owned by the aggregate system itself
    ret->m_path = m_path;            // The hierarchy path is copied with the configuration
    ret->m_date_axis = m_date_axis;  // The fixed time axis is copied with the configuration (the
                                     // axis_mode parameter is copied by System::clone)
    ret->m_adjust_dates =
      m_adjust_dates;  // The external rebalancing day table is copied with the configuration
    if (getMM()) {
        ret->setMM(getMM()->clone());
    }
    // The AF does not hold the running mode any more (it is copied above with m_mode)
    if (m_af) {
        ret->setAF(m_af->clone());
    }
    // m_shadow_sys / m_running_set / m_running_order / m_force_sell_list / m_sub_index /
    // m_open_pending_suggestions / m_kdata_cache are the runtime states, not copied (the shadow
    // registry is rebuilt on the cloned children)
    return ret;
}

void MultiSystem::setDateAxis(const DatetimeList& dates) {
    m_date_axis = dates;
    _clearCycleDates();
}

void MultiSystem::clearDateAxis() {
    m_date_axis.clear();
    _clearCycleDates();
}

void MultiSystem::clearAdjustDates() {
    m_adjust_dates.clear();
    _clearCycleDates();
}

void MultiSystem::setAdjustDates(const DatetimeList& dates) {
    m_adjust_dates.clear();
    for (const auto& date : dates) {
        if (!date.isNull()) {
            m_adjust_dates.insert(date.startOfDay());
        }
    }
    _clearCycleDates();
}

void MultiSystem::_warnAdjustCycleOutOfRange() const {
    string mode = getAdjustMode();
    to_lower(mode);
    int max_days = 0;
    if (mode == "week") {
        // The week period starts on Monday: Saturday/Sunday are never driven
        max_days = 5;
    } else if (mode == "month") {
        max_days = 31;
    } else if (mode == "quarter") {
        max_days = 92;
    } else if (mode == "year") {
        max_days = 366;
    }
    HKU_IF_RETURN(max_days == 0, void());
    const int cycle = getAdjustCycle();
    HKU_IF_RETURN(cycle <= max_days, void());
    HKU_WARN(
      "adjust_cycle {} can never reach a driven day of the {} period (max {} calendar days), "
      "that mode never rebalances! [{}]",
      cycle, mode, max_days, name());
}

Datetime MultiSystem::_periodStart(const Datetime& date) const {
    string mode = m_adjust_mode;
    to_lower(mode);
    if (mode == "week") {
        return date.startOfWeek();
    }
    if (mode == "month") {
        return date.startOfMonth();
    }
    if (mode == "quarter") {
        return date.startOfQuarter();
    }
    if (mode == "year") {
        return date.startOfYear();
    }
    return Datetime();
}

void MultiSystem::_buildCycleDates(const DatetimeList& axis) {
    HKU_IF_RETURN(axis.empty(), void());

    // The rebalancing-day sequence: the explicit (setAdjustDates) table takes precedence; the
    // calendar modes use the same period rule as _isAdjustDate (with a local period cursor seeded
    // from m_last_adjust_period, so a live single-bar replay does not restart the period); the
    // other modes fall back to the close-day counting of m_adjust_cycle
    DatetimeList adjust_dates;
    if (!m_adjust_dates.empty()) {
        for (const auto& d : axis) {
            if (m_adjust_dates.find(d.startOfDay()) != m_adjust_dates.end()) {
                adjust_dates.emplace_back(d);
            }
        }
    } else if (!axis.empty() && !_periodStart(axis.front()).isNull()) {
        Datetime last_period = m_last_adjust_period;
        for (const auto& d : axis) {
            if (_isAdjustDateByPeriod(d, last_period)) {
                adjust_dates.emplace_back(d);
                last_period = _periodStart(d);
            }
        }
    } else {
        const size_t cycle = static_cast<size_t>(m_adjust_cycle > 0 ? m_adjust_cycle : 1);
        for (size_t i = 0, total = axis.size(); i < total; ++i) {
            if ((m_close_day_index + i) % cycle == 0) {
                adjust_dates.emplace_back(axis[i]);
            }
        }
    }

    // Merge with the days declared by the previous runs: the live replay keeps the history
    DatetimeList merged = m_cycle_dates;
    merged.insert(merged.end(), adjust_dates.begin(), adjust_dates.end());
    std::sort(merged.begin(), merged.end());
    merged.erase(std::unique(merged.begin(), merged.end()), merged.end());
    m_cycle_dates = std::move(merged);
}

void MultiSystem::_declareCycleDate(const Datetime& date) {
    // Mode C declares the cycle start day on admission instead (the legacy Portfolio behavior)
    HKU_IF_RETURN(getMode() == "C" || date.isNull(), void());
    // The days stay sorted: a bulk run declares the whole schedule in advance
    HKU_IF_RETURN(std::binary_search(m_cycle_dates.begin(), m_cycle_dates.end(), date), void());

    DatetimeList dates = m_cycle_dates;
    dates.push_back(date);
    std::sort(dates.begin(), dates.end());
    dates.erase(std::unique(dates.begin(), dates.end()), dates.end());
    HKU_IF_RETURN(dates == m_cycle_dates, void());

    m_cycle_dates = std::move(dates);
    _injectCycleDates();
}

void MultiSystem::_clearCycleDates() {
    // Mode C declares the cycle days on admission: they are entry events rather than a schedule and
    // a running sub-system never re-declares, so a configuration change must not retract them
    HKU_IF_RETURN(getMode() == "C", void());

    m_cycle_dates.clear();
    m_last_adjust_period = Datetime();
    // Retract the declared days from the sub-system signals: the whole list is replaced, so an
    // empty list clears them
    DatetimeList empty;
    for (auto& sys : m_sys_list) {
        auto sg = sys->getSG();
        if (sg) {
            sg->setCycleDates(empty);
        } else if (auto* inner = dynamic_cast<MultiSystem*>(sys.get())) {
            // A nested aggregate has no signal of its own, its cycle days live one level down
            inner->_clearCycleDates();
        }
    }
}

void MultiSystem::_injectCycleDates() {
    // Mode C declares the cycle start day on admission (the legacy Portfolio behavior), so the
    // whole run is not pushed down in advance. Only the cycle signals consume the days: skipping
    // the others avoids an extra full-range recalculation on every new declared day
    HKU_IF_RETURN(getMode() == "C" || m_cycle_dates.empty(), void());
    for (auto& sys : m_sys_list) {
        auto sg = sys->getSG();
        if (sg && sg->getParam<bool>("cycle")) {
            sg->setCycleDates(m_cycle_dates);
        } else if (auto* inner = dynamic_cast<MultiSystem*>(sys.get())) {
            // The parent is the scheduler of a nested aggregate (it never runs its own _runAxis)
            if (inner->getMode() != "C") {
                inner->m_cycle_dates = m_cycle_dates;
                inner->_injectCycleDates();
            }
        }
    }
}

Datetime MultiSystem::_getCycleEnd(const Datetime& date) const {
    for (const auto& d : m_cycle_dates) {
        if (d > date) {
            return d;
        }
    }
    return date + Days(1);
}

void MultiSystem::_reduceSubSystemToQuota(const SystemPtr& sys, const Datetime& date, price_t quota,
                                          KQuery::KType ktype, TradeRecordList& out_executed) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    // Scaling a net-short book is not supported yet: covering consumes cash, which the
    // recycle-first order already withdrew. Warn and skip instead of mis-calibrating.
    if (_subtreeHasNetShort(sys)) {
        if (!m_short_reduce_warned) {
            m_short_reduce_warned = true;
            HKU_WARN("Net-short quota reduction is not supported yet (sub {}), skipped!",
                     sys->name());
        }
        return;
    }

    // The sub cash has been recycled before (trade_on_close=true), so the net assets measure the
    // holding value; with trade_on_close=false that recycle is skipped and the idle cash must be
    // excluded too
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t sub_total = funds.net_assets();
    price_t reduce_base = m_trade_on_close ? sub_total : (sub_total - funds.cash);
    if (sys->isComposite()) {
        auto* inner = dynamic_cast<MultiSystem*>(sys.get());
        const bool delegatable = inner && inner->getMode() == "B" && inner->getAF();
        if (delegatable && quota > 0.0) {
            // The inner manages its own leaf ledgers: delegate the budget and let it reduce its
            // leaves (the FOF/MOM semantics, both book levels stay matched)
            inner->m_external_quota = quota;
            inner->m_external_quota_set = true;
        } else {
            if (!delegatable && inner && inner->getMode() == "B" && quota > 0.0 &&
                !m_external_delegate_warned) {
                m_external_delegate_warned = true;
                HKU_WARN(
                  "The nested mode B {} has no AF to consume the delegated quota, fall back to "
                  "the direct pro rata reduction!",
                  inner->name());
            }
            if (quota > 0.0 && reduce_base > quota) {
                // Reduce the composite account pro rata over its leaf holdings
                _reduceCompositeToQuota(sys, date, reduce_base - quota, ktype, out_executed);
            }
        }
        return;
    }
    if (reduce_base <= quota || quota <= 0.0) {
        return;
    }

    // Above the quota: reduce the position by the over quota market value (the master algorithm)
    price_t need_back_funds = reduce_base - quota;
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
    // A cross-day retry leg may already be planned for this stock: deduct it so the same excess
    // is not sold twice
    double planned = 0.0;
    for (const auto& a : m_pending_calibration) {
        if (a.sys == sys && a.kind == CalibrationKind::SUB_SELL && a.stock == stock) {
            planned += a.number;
        }
    }
    need_back_num = std::max(0.0, need_back_num - planned);
    if (need_back_num <= 0.0) {
        return;
    }

    if (!m_trade_on_close) {
        // Defer the whole reduction (sub and parent side) to the next open, capping the parent
        // leg by the parent holding (a parent side without shares can never fill)
        m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stock, need_back_num});
        double parent_num = std::min(need_back_num, m_tm->getPosition(date, stock).number);
        if (parent_num > 0.0) {
            m_pending_calibration.push_back({CalibrationKind::PARENT_SELL, sys, stock, parent_num});
        }
        m_pending_calibration.push_back({CalibrationKind::SUB_CHECKOUT, sys, Stock(), 0.0});
        return;
    }

    price_t close_price = _getClosePrice(date, stock);
    if (close_price <= 0.0) {
        // Suspended at the close: queue the legs and let the executor retry them
        m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stock, need_back_num});
        double parent_num = std::min(need_back_num, m_tm->getPosition(date, stock).number);
        if (parent_num > 0.0) {
            m_pending_calibration.push_back({CalibrationKind::PARENT_SELL, sys, stock, parent_num});
        }
        m_pending_calibration.push_back({CalibrationKind::SUB_CHECKOUT, sys, Stock(), 0.0});
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
    // The parent sells what its own book actually holds: asking for more than that would be
    // rejected as a whole, leaving the divergence uncorrected
    double parent_num = std::min(need_back_num, m_tm->getPosition(date, stock).number);
    TradeRecord parent_tr;
    if (parent_num > 0.0) {
        parent_tr = m_tm->sell(date, stock, close_price, parent_num, 0.0, 0.0, close_price,
                               PART_PORTFOLIO, "MultiSystem");
    }
    if (!parent_tr.isNull()) {
        out_executed.push_back(parent_tr);
    }
}

price_t MultiSystem::_injectSubSystemGap(const SystemPtr& sys, const Datetime& date, price_t quota,
                                         KQuery::KType ktype, price_t parent_cash_limit) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, 0.0, "Sub system has no trade manager! {}", sys->name());

    // The quota cash is a virtual budget: the injection only credits the sub ledger, the parent TM
    // is never debited. The remaining-cash cap (accumulated over the injections of this rebalancing
    // day) is what keeps the mirrored parent buys fundable and the two books matched

    // Injecting into a net-short book is not supported yet: warn and skip
    if (_subtreeHasNetShort(sys)) {
        if (!m_short_inject_warned) {
            m_short_inject_warned = true;
            HKU_WARN("Net-short quota injection is not supported yet (sub {}), skipped!",
                     sys->name());
        }
        return 0.0;
    }

    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t sub_total = funds.net_assets();
    if (sub_total >= quota || quota <= 0.0) {
        return 0.0;
    }

    // Below the quota: inject the cash gap (limited by the remaining free cash of the parent pool,
    // so the sum of the injections within one rebalancing day never exceeds what the parent can
    // actually fund -- the two books must stay matched)
    price_t diff = quota - sub_total;
    if (diff > parent_cash_limit) {
        // The gap is truncated by the remaining parent cash: without a notice the under-funded
        // sub-system would silently stay below its quota (once per instance, to avoid flooding)
        if (!m_inject_truncated_warned) {
            m_inject_truncated_warned = true;
            HKU_WARN(
              "{}: the mode B gap injection of the sub {} is truncated by the parent cash "
              "({:.2f} available < {:.2f} needed)!",
              name(), sys->name(), parent_cash_limit, diff);
        }
        diff = parent_cash_limit;
    }
    if (diff > 0.0) {
        sub_tm->checkin(date, diff);
        return diff;
    }
    return 0.0;
}

void MultiSystem::_clearSubSystem(const SystemPtr& sys, const Datetime& date, KQuery::KType ktype) {
    HKU_IF_RETURN(!sys, void());
    _purgePendingFor(sys);
    if (sys->isComposite()) {
        for (const auto& child : sys->getSubSystemList()) {
            _clearSubSystem(child, date, ktype);
        }
    }
    if (auto* ms = dynamic_cast<MultiSystem*>(sys.get())) {
        ms->_discardOpenPending("the sub-system is cleared");
        ms->m_external_quota = 0.0;
        ms->m_external_quota_set = false;
    }
    sys->clearDelayBuyRequest();
    sys->clearDelaySellRequest();
    _clearSubSystemLedger(sys, date, ktype);
}

void MultiSystem::_clearSubSystemAccounts(const SystemPtr& sys, const Datetime& date,
                                          KQuery::KType ktype) {
    HKU_IF_RETURN(!sys, void());
    if (sys->isComposite()) {
        for (const auto& child : sys->getSubSystemList()) {
            _clearSubSystemAccounts(child, date, ktype);
        }
    }
    if (auto* ms = dynamic_cast<MultiSystem*>(sys.get())) {
        ms->_discardOpenPending("the sub-system is removed");
        ms->m_external_quota = 0.0;
        ms->m_external_quota_set = false;
    }
    sys->clearDelayBuyRequest();
    sys->clearDelaySellRequest();
    TMPtr tm = sys->getTM();
    if (tm && tm == m_tm) {
        return;  // the shared account itself is cleared by the parent own sells
    }
    _clearSubSystemLedger(sys, date, ktype);
}

void MultiSystem::_discardOpenPending(const string& reason) {
    if (!m_open_pending_suggestions.empty()) {
        HKU_WARN("{}: discard {} suggestion(s) queued for the next open ({})", name(),
                 m_open_pending_suggestions.size(), reason);
        m_open_pending_suggestions.clear();
        m_open_pending_date = Datetime();
        m_open_pending_assets = 0.0;
    }
    m_pending_calibration.clear();
    m_pending_calibration_date = Datetime();
    m_pending_calibration_assets = 0.0;
    m_pending_calibration_retry = false;
    m_open_pending_retry = false;
    m_open_pending_adjust = false;
}

void MultiSystem::_pushCheckoutOnce(const SystemPtr& sys) {
    auto& v = m_pending_calibration;
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](const CalibrationAction& a) {
                               return a.sys == sys && a.kind == CalibrationKind::SUB_CHECKOUT;
                           }),
            v.end());
    v.push_back({CalibrationKind::SUB_CHECKOUT, sys, Stock(), 0.0});
}

void MultiSystem::_purgePendingFor(const SystemPtr& sys) {
    std::set<Stock> stocks;
    const Stock& own = sys->getStock();
    if (own.isNull()) {
        _collectLeafStocks(sys, stocks);
    } else {
        stocks.insert(own);
    }
    auto& v = m_open_pending_suggestions;
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](const TradeSuggestion& s) {
                               // A mirror suggestion may not carry the owner: fall back to the
                               // stock set of the cleared sub-system
                               return s.sys == sys || (!s.sys && stocks.count(s.stock) > 0);
                           }),
            v.end());
    if (v.empty()) {
        m_open_pending_date = Datetime();
        m_open_pending_assets = 0.0;
        m_open_pending_retry = false;
        m_open_pending_adjust = false;
    }
}

void MultiSystem::_executePendingCalibration(const Datetime& date, KQuery::KType ktype,
                                             TradeRecordList& out) {
    HKU_IF_RETURN(m_pending_calibration.empty(), void());
    std::vector<CalibrationAction> retry;
    // Phase 1: the sub / parent sells and the checkouts
    for (const auto& a : m_pending_calibration) {
        if (a.kind == CalibrationKind::INJECT) {
            continue;
        }
        if (a.kind == CalibrationKind::SUB_CHECKOUT) {
            if (!a.sys || !a.sys->getTM()) {
                continue;
            }
            // The checkout waits while a sell leg of the same sub-system is still retried: the
            // sell proceeds are not in the sub ledger yet
            bool waiting = std::any_of(retry.begin(), retry.end(), [&](const CalibrationAction& r) {
                return r.sys == a.sys && (r.kind == CalibrationKind::SUB_SELL ||
                                          r.kind == CalibrationKind::SUB_COVER);
            });
            if (waiting) {
                retry.push_back(a);
                continue;
            }
            FundsRecord f = a.sys->getTM()->getFunds(date, ktype);
            if (f.cash > 0.0) {
                a.sys->getTM()->checkout(date, f.cash);
            }
            continue;
        }
        // A sub leg is judged and capped by the initiating sub book, a parent leg by the parent
        // book (the parent side can only sell what the parent account actually holds)
        TMPtr own_tm = (a.sys && a.sys->getTM()) ? a.sys->getTM() : m_tm;
        price_t price = _getOpenPrice(date, a.stock);
        if (price <= 0.0) {
            // No quote: the leg is left to the next session -- unless the holding is already gone
            // (e.g. the instrument was delisted and force-cleared while the leg waited): such a
            // dead leg would retry forever and block every future injection
            const bool is_parent =
              (a.kind == CalibrationKind::PARENT_SELL || a.kind == CalibrationKind::PARENT_COVER);
            const bool is_long =
              (a.kind == CalibrationKind::SUB_SELL || a.kind == CalibrationKind::PARENT_SELL);
            TMPtr judge_tm = is_parent ? m_tm : own_tm;
            const double own = is_long ? judge_tm->getPosition(date, a.stock).number
                                       : judge_tm->getShortPosition(a.stock).number;
            if (own <= 0.0) {
                continue;
            }
            retry.push_back(a);
            continue;
        }
        if (a.kind == CalibrationKind::SUB_SELL || a.kind == CalibrationKind::SUB_COVER) {
            if (!a.sys || !a.sys->getTM()) {
                continue;
            }
            TMPtr sub_tm = a.sys->getTM();
            if (a.kind == CalibrationKind::SUB_SELL) {
                double num = std::min(a.number, sub_tm->getPosition(date, a.stock).number);
                if (num > 0.0) {
                    sub_tm->sell(date, a.stock, price, num, 0.0, 0.0, price, PART_PORTFOLIO,
                                 "MultiSystem");
                }
            } else {
                double num = std::min(a.number, sub_tm->getShortPosition(a.stock).number);
                if (num > 0.0) {
                    sub_tm->buyShort(date, a.stock, price, num, 0.0, 0.0, price, PART_PORTFOLIO,
                                     "MultiSystem");
                }
            }
            continue;
        }
        if (a.kind == CalibrationKind::PARENT_SELL) {
            // The parent leg carries the sub's intended quantity, capped by the parent holding
            // (for a reduction leg the sub book already sold its part in the same phase; for a
            // liquidation leg this leg runs before the sub legs)
            double need = a.number;
            double num = std::min(need, m_tm->getPosition(date, a.stock).number);
            if (num > 0.0) {
                TradeRecord tr = m_tm->sell(date, a.stock, price, num, 0.0, 0.0, price,
                                            PART_PORTFOLIO, "MultiSystem");
                if (!tr.isNull()) {
                    out.push_back(tr);
                }
            }
            if (need > num) {
                // The parent has no quantity left: the residual can never fill (keeping it would
                // later sell shares the parent acquires for somebody else)
                if (m_tm->getPosition(date, a.stock).number <= 0.0) {
                    continue;
                }
                CalibrationAction residual = a;
                residual.number = need - num;
                retry.push_back(residual);
            }
        } else {
            double need = a.number;
            double num = std::min(need, m_tm->getShortPosition(a.stock).number);
            if (num > 0.0) {
                TradeRecord tr = m_tm->buyShort(date, a.stock, price, num, 0.0, 0.0, price,
                                                PART_PORTFOLIO, "MultiSystem");
                if (!tr.isNull()) {
                    out.push_back(tr);
                }
            }
            if (need > num) {
                if (m_tm->getShortPosition(a.stock).number <= 0.0) {
                    continue;
                }
                CalibrationAction residual = a;
                residual.number = need - num;
                retry.push_back(residual);
            }
        }
    }

    // Phase 2: the deferred injections, funded by the actual cash after the sells. While some
    // sell legs still wait for a quote, the injections wait with them: the freed cash is part of
    // their funding, injecting now would truncate the quota
    if (retry.empty()) {
        price_t cash_left = m_tm->getFunds(date, ktype).cash;
        for (const auto& a : m_pending_calibration) {
            if (a.kind != CalibrationKind::INJECT || !a.sys || cash_left <= 0.0) {
                continue;
            }
            cash_left -= _injectSubSystemGap(a.sys, date, a.quota, ktype, cash_left);
        }
    } else {
        for (const auto& a : m_pending_calibration) {
            if (a.kind == CalibrationKind::INJECT) {
                retry.push_back(a);
            }
        }
    }

    if (!retry.empty()) {
        m_pending_calibration = std::move(retry);
        m_pending_calibration_retry = true;
    } else {
        m_pending_calibration.clear();
        m_pending_calibration_date = Datetime();
        m_pending_calibration_assets = 0.0;
        m_pending_calibration_retry = false;
    }
}

void MultiSystem::_clearSubSystemLedger(const SystemPtr& sys, const Datetime& date,
                                        KQuery::KType ktype) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());

    // Only the sub ledger is cleared: the cash checkout has no parent counterpart, the parent side
    // is liquidated by its own sell (see the unselected handling), so the two books stay matched

    Stock stock = sys->getStock();
    if (!m_trade_on_close) {
        // Defer to the next open: the legs of the whole rebalancing share one price basis
        std::set<Stock> stocks;
        if (stock.isNull()) {
            _collectLeafStocks(sys, stocks);
        } else {
            stocks.insert(stock);
        }
        for (const auto& stk : stocks) {
            double num = sub_tm->getPosition(date, stk).number;
            if (num > 0.0) {
                m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stk, num});
            }
            double short_num = sub_tm->getShortPosition(stk).number;
            if (short_num > 0.0) {
                m_pending_calibration.push_back({CalibrationKind::SUB_COVER, sys, stk, short_num});
            }
        }
        m_pending_calibration.push_back({CalibrationKind::SUB_CHECKOUT, sys, Stock(), 0.0});
        return;
    }

    bool deferred = false;
    if (stock.isNull()) {
        // A composite has no single instrument: sell its leaf holdings directly on the sub ledger
        std::set<Stock> stocks;
        _collectLeafStocks(sys, stocks);
        for (const auto& stk : stocks) {
            price_t p = _getClosePrice(date, stk);
            if (p <= 0.0) {
                // Suspended: queue the legs so the checkout waits for the sell proceeds
                double num = sub_tm->getPosition(date, stk).number;
                if (num > 0.0) {
                    m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stk, num});
                    deferred = true;
                }
                double short_num = sub_tm->getShortPosition(stk).number;
                if (short_num > 0.0) {
                    m_pending_calibration.push_back(
                      {CalibrationKind::SUB_COVER, sys, stk, short_num});
                    deferred = true;
                }
                continue;
            }
            double num = sub_tm->getPosition(date, stk).number;
            if (num > 0.0) {
                sub_tm->sell(date, stk, p, num, 0.0, 0.0, p, PART_PORTFOLIO, "MultiSystem");
            }
            if (sub_tm->getShortPosition(stk).number > 0.0) {
                sub_tm->buyShort(date, stk, p, MAX_DOUBLE, 0.0, 0.0, p, PART_PORTFOLIO,
                                 "MultiSystem");
            }
        }
    } else {
        price_t p = _getClosePrice(date, stock);
        if (p <= 0.0) {
            double num = sub_tm->getPosition(date, stock).number;
            if (num > 0.0) {
                m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stock, num});
                deferred = true;
            }
            double short_num = sub_tm->getShortPosition(stock).number;
            if (short_num > 0.0) {
                m_pending_calibration.push_back(
                  {CalibrationKind::SUB_COVER, sys, stock, short_num});
                deferred = true;
            }
        } else {
            PositionRecord position = sub_tm->getPosition(date, stock);
            // A long is sold and a short is covered (System::_sellForce flattens whichever side)
            if (position.number > 0.0 || sub_tm->getShortPosition(stock).number > 0.0) {
                sys->sellForceOnClose(date, MAX_DOUBLE, PART_PORTFOLIO);
            }
        }
    }
    if (deferred) {
        m_pending_calibration.push_back({CalibrationKind::SUB_CHECKOUT, sys, Stock(), 0.0});
        return;
    }
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    if (funds.cash > 0.0) {
        sub_tm->checkout(date, funds.cash);
    }
}

void MultiSystem::_liquidateParent(const SystemPtr& sys, const Datetime& date,
                                   TradeRecordList& out) {
    std::set<Stock> stocks;
    const Stock& own = sys->getStock();
    if (own.isNull()) {
        _collectLeafStocks(sys, stocks);
    } else {
        stocks.insert(own);
    }
    // The child own book caps the parent sell: with several children holding the same instrument
    // the parent total is not attributable to the removed one alone
    TMPtr own_tm = sys->getTM();
    const bool has_own_book = own_tm && own_tm != m_tm;
    for (const auto& stk : stocks) {
        price_t price = _getClosePrice(date, stk);
        double long_num = m_tm->getPosition(date, stk).number;
        double short_num = m_tm->getShortPosition(stk).number;
        if (has_own_book) {
            long_num = std::min(long_num, own_tm->getPosition(date, stk).number);
            short_num = std::min(short_num, own_tm->getShortPosition(stk).number);
        }
        if (!m_trade_on_close) {
            // Enqueue before the quote check: the executor re-checks the quote and the holding,
            // so a suspended close must not drop the parent side while the sub side is queued
            if (long_num > 0.0) {
                m_pending_calibration.push_back({CalibrationKind::PARENT_SELL, sys, stk, long_num});
            }
            if (short_num > 0.0) {
                m_pending_calibration.push_back(
                  {CalibrationKind::PARENT_COVER, sys, stk, short_num});
            }
            continue;
        }
        if (price <= 0.0) {
            // Suspended at the close even with trade_on_close=true: defer the parent legs, the
            // executor retries them at the next open
            if (long_num > 0.0) {
                m_pending_calibration.push_back({CalibrationKind::PARENT_SELL, sys, stk, long_num});
            }
            if (short_num > 0.0) {
                m_pending_calibration.push_back(
                  {CalibrationKind::PARENT_COVER, sys, stk, short_num});
            }
            continue;
        }
        if (long_num > 0.0) {
            TradeRecord tr = m_tm->sell(date, stk, price, long_num, 0.0, 0.0, price, PART_PORTFOLIO,
                                        "MultiSystem");
            if (!tr.isNull()) {
                out.push_back(tr);
            }
        }
        if (short_num > 0.0) {
            // Cover the short book (MAX_DOUBLE is capped at the actual short)
            TradeRecord tr = m_tm->buyShort(date, stk, price, short_num, 0.0, 0.0, price,
                                            PART_PORTFOLIO, "MultiSystem");
            if (!tr.isNull()) {
                out.push_back(tr);
            }
        }
    }
}

void MultiSystem::_reduceCompositeToQuota(const SystemPtr& sys, const Datetime& date,
                                          price_t need_back_funds, KQuery::KType ktype,
                                          TradeRecordList& out_executed) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());
    if (need_back_funds <= 0.0) {
        return;
    }

    std::set<Stock> stocks;
    _collectLeafStocks(sys, stocks);
    std::map<Stock, double> values;
    std::map<Stock, double> remaining;
    double total_value = 0.0;
    for (const auto& stk : stocks) {
        double num = sub_tm->getPosition(date, stk).number;
        price_t p = _getClosePrice(date, stk);
        if (p <= 0.0) {
            // Suspended: fall back to the last available close, so the pro-rata reduction covers
            // the whole composite instead of liquidating only the quoted leaves
            p = stk.getMarketValue(date, ktype);
        }
        if (num <= 0.0 || p <= 0.0) {
            continue;
        }
        double v = num * p * stk.unit();
        values[stk] = v;
        remaining[stk] = num;
        total_value += v;
    }
    if (total_value <= 0.0) {
        return;
    }

    // The queued legs of this sub-system count as sold for the residual base only: the pro-rata
    // ratio still uses the full gap, otherwise the share deduction below would subtract the same
    // legs twice (the round would undersell when the queued legs cover only part of the gap)
    const price_t need_gap = need_back_funds;
    std::map<Stock, double> planned_nums;
    double planned_value = 0.0;
    for (const auto& [stk, value] : values) {
        double hold = sub_tm->getPosition(date, stk).number;
        double sum = 0.0;
        for (const auto& a : m_pending_calibration) {
            if (a.sys == sys && a.kind == CalibrationKind::SUB_SELL && a.stock == stk) {
                sum += a.number;
            }
        }
        double planned = std::min(sum, hold);
        if (planned <= 0.0) {
            continue;
        }
        price_t p = _getClosePrice(date, stk);
        if (p <= 0.0) {
            p = stk.getMarketValue(date, ktype);
        }
        if (p > 0.0) {
            planned_nums[stk] = planned;
            planned_value += planned * p * stk.unit();
        }
    }

    double sold_value = 0.0;
    bool deferred = false;

    auto sell_part = [&](const Stock& stk, double num, price_t close_price) {
        if (num <= 0.0) {
            return;
        }
        remaining[stk] -= num;
        sold_value += num * close_price * stk.unit();
        if (!m_trade_on_close) {
            m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stk, num});
            double parent_num = std::min(num, m_tm->getPosition(date, stk).number);
            if (parent_num > 0.0) {
                m_pending_calibration.push_back(
                  {CalibrationKind::PARENT_SELL, sys, stk, parent_num});
            }
            return;
        }
        sub_tm->sell(date, stk, close_price, num, 0.0, 0.0, close_price, PART_PORTFOLIO,
                     "MultiSystem");
        double parent_num = std::min(num, m_tm->getPosition(date, stk).number);
        if (parent_num > 0.0) {
            TradeRecord tr = m_tm->sell(date, stk, close_price, parent_num, 0.0, 0.0, close_price,
                                        PART_PORTFOLIO, "MultiSystem");
            if (!tr.isNull()) {
                out_executed.push_back(tr);
            }
        }
    };

    if (planned_value >= need_gap) {
        // The queued legs already cover the whole gap: trim them to the gap instead of adding
        // pro-rata sells on top (they would push the sub-system below its quota)
        double keep_ratio = need_gap / planned_value;
        auto trim_kind = [&](const Stock& stk, CalibrationKind kind, double target) {
            double left = target;
            for (auto& a : m_pending_calibration) {
                if (a.sys != sys || a.stock != stk || a.kind != kind) {
                    continue;
                }
                if (left <= 0.0) {
                    a.number = 0.0;
                } else if (a.number > left) {
                    a.number = left;
                    left = 0.0;
                } else {
                    left -= a.number;
                }
            }
        };
        for (const auto& [stk, value] : values) {
            double hold = sub_tm->getPosition(date, stk).number;
            double min_num = stk.minTradeNumber();
            price_t p = _getClosePrice(date, stk);
            if (p <= 0.0) {
                p = stk.getMarketValue(date, ktype);
            }
            if (min_num <= 0.0 || p <= 0.0 || hold <= 0.0) {
                continue;
            }
            double sum_sub = 0.0;
            double sum_parent = 0.0;
            for (const auto& a : m_pending_calibration) {
                if (a.sys != sys || a.stock != stk) {
                    continue;
                }
                if (a.kind == CalibrationKind::SUB_SELL) {
                    sum_sub += a.number;
                } else if (a.kind == CalibrationKind::PARENT_SELL) {
                    sum_parent += a.number;
                }
            }
            double sub_target =
              std::floor(std::min(sum_sub, hold) * keep_ratio / min_num) * min_num;
            double parent_target = std::floor(sum_parent * keep_ratio / min_num) * min_num;
            parent_target = std::min(parent_target, sub_target);
            trim_kind(stk, CalibrationKind::SUB_SELL, sub_target);
            trim_kind(stk, CalibrationKind::PARENT_SELL, parent_target);
            remaining[stk] -= sub_target;
            sold_value += sub_target * p * stk.unit();
            if (sub_target > 0.0) {
                deferred = true;
            }
        }
        m_pending_calibration.erase(
          std::remove_if(m_pending_calibration.begin(), m_pending_calibration.end(),
                         [&](const CalibrationAction& a) {
                             return a.sys == sys && !a.stock.isNull() &&
                                    (a.kind == CalibrationKind::SUB_SELL ||
                                     a.kind == CalibrationKind::PARENT_SELL) &&
                                    a.number <= 0.0;
                         }),
          m_pending_calibration.end());
    } else {
        need_back_funds = std::max(0.0, need_back_funds - planned_value);
        for (const auto& kv : planned_nums) {
            remaining[kv.first] -= kv.second;
        }

        double ratio = std::min(1.0, need_gap / total_value);

        for (const auto& [stk, value] : values) {
            price_t close_price = _getClosePrice(date, stk);
            double hold = sub_tm->getPosition(date, stk).number;
            double min_num = stk.minTradeNumber();
            if (min_num <= 0.0) {
                continue;
            }
            // A cross-day retry leg may already be planned for this stock: deduct it so the same
            // excess is not sold twice
            double planned = 0.0;
            if (auto it = planned_nums.find(stk); it != planned_nums.end()) {
                planned = it->second;
            }
            if (close_price <= 0.0) {
                // Suspended: queue the pro-rata leg so the excess is sold at the next quoted open
                double num = std::floor(hold * ratio / min_num) * min_num;
                num = std::min(num, hold);
                num = std::max(0.0, num - planned);
                num = std::floor(num / min_num) * min_num;
                if (num > 0.0) {
                    m_pending_calibration.push_back({CalibrationKind::SUB_SELL, sys, stk, num});
                    double parent_num = std::min(num, m_tm->getPosition(date, stk).number);
                    if (parent_num > 0.0) {
                        m_pending_calibration.push_back(
                          {CalibrationKind::PARENT_SELL, sys, stk, parent_num});
                    }
                    remaining[stk] -= num;
                    sold_value += value * num / hold;
                    deferred = true;
                }
                continue;
            }
            double num = std::floor(value * ratio / close_price / min_num) * min_num;
            num = std::min(num, hold);
            num = std::max(0.0, num - planned);
            num = std::floor(num / min_num) * min_num;
            sell_part(stk, num, close_price);
        }
    }

    // The lot rounding leaves a residual: sell one more lot from the largest remaining holdings
    // until the excess is covered (or there is nothing left to sell)
    double residual = need_back_funds - sold_value;
    bool progressed = true;
    while (residual > 0.0 && progressed) {
        progressed = false;
        std::vector<std::pair<Stock, double>> candidates;
        for (const auto& item : remaining) {
            price_t p = _getClosePrice(date, item.first);
            candidates.emplace_back(item.first, item.second * p * item.first.unit());
        }
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        for (const auto& item : candidates) {
            const Stock& stk = item.first;
            price_t close_price = _getClosePrice(date, stk);
            double min_num = stk.minTradeNumber();
            if (close_price <= 0.0 || min_num <= 0.0) {
                continue;
            }
            double held = remaining[stk];
            if (held < min_num) {
                continue;
            }
            double num = std::min(min_num, held);
            sell_part(stk, num, close_price);
            residual -= num * close_price * stk.unit();
            progressed = true;
            if (residual <= 0.0) {
                break;
            }
        }
    }
    bool has_pending_sell = std::any_of(
      m_pending_calibration.begin(), m_pending_calibration.end(), [&](const CalibrationAction& a) {
          return a.sys == sys &&
                 (a.kind == CalibrationKind::SUB_SELL || a.kind == CalibrationKind::SUB_COVER);
      });
    if (!m_trade_on_close || deferred || has_pending_sell) {
        _pushCheckoutOnce(sys);
        return;
    }
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    if (funds.cash > 0.0) {
        sub_tm->checkout(date, funds.cash);
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
            } else if (m == "month") {
                adjust = (date.day() == cycle);  // The cycle-th day of every month
            } else if (m == "quarter") {
                adjust = (date == date.startOfQuarter() + Days(cycle - 1));  // The cycle-th day
            } else {                                                         // year
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
    // The driving axis selection (the axis_mode parameter, see MultiSystem.h):
    //   "kdata"    -- the date sequence of the input KData is used as the axis (the default,
    //   keeping the existing behavior) "calendar" -- the fixed date table (e.g. the market-wide
    //   trading calendar) injected by setDateAxis() is used as the axis;
    //                 the input KData degenerates into the query/ktype and the price query context
    //                 (runMoment/_closePhase only take its query)
    string axis_mode = tryGetParam<string>("axis_mode", "kdata");
    bool use_calendar_axis = (axis_mode == "calendar");
    if (use_calendar_axis && m_date_axis.empty()) {
        HKU_WARN("axis_mode=calendar but date axis is empty, fallback to kdata axis! [{}]", name());
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

    // Consistent with master: when ktype is not the daily line, it is only allowed when adjust_mode
    // is query/day (the calendar axis is a daily line sequence)
    string mode = getAdjustMode();
    to_lower(mode);
    HKU_CHECK(mode == "query" || mode == "day" || query.kType() == KQuery::DAY,
              "The kType of query must be DAY when adjust_mode is not \"query\"! [{}]", name());

    // The driving axis: the explicitly injected fixed time axis takes precedence (respecting the
    // injection of the axis_mode=calendar users), otherwise the market trading calendar is used
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
    HKU_CHECK(m_tm, "Not setTradeManager! {}", name());
    _refreshRunConfig();

    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    // The driving context is (re)built after the reset: a full reset wipes it, and without it the
    // sub-system trading objects could not be rebuilt and nothing would be driven at all
    m_kdata = kdata;
    m_stock = m_kdata.getStock();
    m_kdata_cache.clear();
    m_kdata_cache_mark = Datetime();

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

    // The driven subset of the axis: only the bars after the account window are driven, and the
    // rebalancing-day expansion and the declared cycle days are built on that very subset, so that
    // they agree with the close-day counter behind the rebalancing test
    DatetimeList driven_axis;
    if (axis) {
        // Driven by the fixed time axis: the dates on the axis may not exist in the input KData
        // (the suspended/non-trading days do not constitute a gap)
        for (const auto& dt : *axis) {
            if (dt >= tm_init_datetime && dt >= tm_last_datetime) {
                driven_axis.push_back(dt);
            }
        }
    } else {
        // The aggregate system drives all the sub-systems on the fully aligned time axis
        size_t total = m_kdata.size();
        auto const* ks = m_kdata.data();
        for (size_t i = 0; i < total; ++i) {
            if (ks[i].datetime >= tm_init_datetime && ks[i].datetime >= tm_last_datetime) {
                driven_axis.push_back(ks[i].datetime);
            }
        }
    }
    // The cycle days are declared on the driven axis before the sub-system trading objects are
    // (re)calculated (the external setAdjustDates table takes precedence, see _buildCycleDates)
    _buildCycleDates(driven_axis);

    readyForRun();

    // Notify SE of the actually running system list (mapped with the prototypes), and start its
    // calculation
    if (m_se) {
        m_se->calculate(m_sys_list, m_kdata.getQuery());
    }

    for (const auto& dt : driven_axis) {
        runMoment(dt);
    }
    m_calculated = true;
}

TradeSuggestionList MultiSystem::_toSuggestions(const SystemPtr& sys, const TradeRecordList& trades,
                                                const FundsRecord& funds_before,
                                                const Datetime& datetime) const {
    TradeSuggestionList result;
    // The "before-trade" fund benchmark of the sub-system (to prevent division by zero): used to
    // calculate the three ratios of the suggestion. The denominator is the net assets so that a
    // short book keeps a meaningful ratio.
    double base_assets = funds_before.net_assets();
    double base_cash = funds_before.cash;

    // One suggestion per executed leg (per instrument and TM primitive), never folded into one
    // signed net delta: the leg structure is the only thing the parent can mirror 1:1. A folded
    // net delta that crosses zero (e.g. a nested aggregate selling its long leg and opening a
    // short leg on the same instrument in one bar) is not expressible by any single TM primitive
    // -- the reconstructed one-leg order would sell more than the parent holds and be rejected.
    enum LegAction { LEG_BUY = 0, LEG_SELL, LEG_COVER, LEG_OPENSHORT };
    struct LegAcc {
        double number{0.0};
        price_t price{0.0};
    };
    std::map<std::pair<Stock, int>, LegAcc> legs;
    std::set<Stock> stocks;
    for (const auto& tr : trades) {
        int action;
        switch (tr.business) {
            case BUSINESS_BUY:
                action = LEG_BUY;
                break;
            case BUSINESS_SELL:
                action = LEG_SELL;
                break;
            case BUSINESS_BUY_SHORT:
                action = LEG_COVER;
                break;
            case BUSINESS_SELL_SHORT:
                action = LEG_OPENSHORT;
                break;
            default:
                continue;
        }
        if (tr.stock.isNull() || tr.number == 0.0) {
            continue;
        }
        LegAcc& acc = legs[{tr.stock, action}];
        acc.number += tr.number;
        if (tr.realPrice > 0.0) {
            acc.price = tr.realPrice;
        }
        stocks.insert(tr.stock);
    }

    // The post-trade book of every traded instrument (getPosition already reflects the just
    // executed trades), used for the CLEAR detection
    std::map<Stock, std::pair<double, double>> post_book;  // stock -> (long, short)
    if (sys->getTM()) {
        for (const auto& stock : stocks) {
            post_book[stock] = {sys->getTM()->getPosition(datetime, stock).number,
                                sys->getTM()->getShortPosition(stock).number};
        }
    }

    for (const auto& kv : legs) {
        const Stock& stock = kv.first.first;
        int action = kv.first.second;
        const LegAcc& acc = kv.second;
        if (stock.isNull() || acc.number == 0.0) {
            continue;
        }
        TradeSuggestion s;
        s.stock = stock;
        s.sys = sys;
        s.number = std::fabs(acc.number);
        s.plan_price = acc.price;
        s.plan_cash = s.number * s.plan_price * stock.unit();
        s.from = PART_SYSTEM;
        bool flattens = false;
        switch (action) {
            case LEG_BUY:
                s.type = SuggestionType::BUY;
                s.short_side = false;
                break;
            case LEG_SELL:
                s.short_side = false;
                flattens = post_book.count(stock) && post_book[stock].first == 0.0;
                s.type = flattens ? SuggestionType::CLEAR : SuggestionType::SELL;
                break;
            case LEG_COVER:
                s.short_side = true;
                flattens = post_book.count(stock) && post_book[stock].second == 0.0;
                s.type = flattens ? SuggestionType::CLEAR : SuggestionType::BUY;
                break;
            case LEG_OPENSHORT:
                s.type = SuggestionType::SELL;
                s.short_side = true;
                break;
        }
        // The three ratios: the sub-system before-trade net assets are the denominator. The sign
        // of assets_ratio encodes the direction (> 0 buy/cover, < 0 sell/open-short);
        // target_position_ratio carries the signed net target level of the instrument -- attached
        // to the first leg of the stock only, so that a per-suggestion summation does not double
        // the level of a two-leg crossing
        if (base_assets > 0.0 && s.plan_price > 0.0) {
            double ratio = s.plan_cash / base_assets;
            if (ratio > 1.0) {
                ratio = 1.0;
            } else if (ratio < -1.0) {
                ratio = -1.0;
            }
            s.assets_ratio = (action == LEG_BUY || action == LEG_COVER) ? ratio : -ratio;
            bool first_leg_of_stock =
              std::none_of(result.begin(), result.end(),
                           [&stock](const TradeSuggestion& x) { return x.stock == stock; });
            if (first_leg_of_stock) {
                double long_pos = post_book.count(stock) ? post_book[stock].first : 0.0;
                double short_pos = post_book.count(stock) ? post_book[stock].second : 0.0;
                s.target_position_ratio =
                  (long_pos - short_pos) * s.plan_price * stock.unit() / base_assets;
            }
        }
        if (base_cash > 0.0) {
            s.cash_ratio = s.plan_cash / base_cash;
        }
        result.push_back(s);
    }
    return result;
}

void MultiSystem::_queueOpenPending(const Datetime& date, const TradeSuggestionList& suggestions,
                                    bool is_adjust) {
    HKU_WARN_IF_RETURN(suggestions.empty(), void(), "Empty suggestions!");
    m_open_pending_assets =
      m_tm ? m_tm->getFunds(date, m_kdata.getQuery().kType()).total_assets() : 0.0;
    // Keyed by the driving day: a repeat drive of the same close day replaces the queued batch, so
    // that a double-driven session cannot silently double the orders executed at the next open
    if (!m_open_pending_date.isNull() && date == m_open_pending_date) {
        if (m_open_pending_retry) {
            return;  // keep the batch with the merged retry legs (a re-drive is idempotent)
        }
        m_open_pending_suggestions = suggestions;
        return;
    }
    // A batch left over from an earlier day was never consumed at an open (a session that only
    // drives the close): drop it, keeping only the latest batch. Warn once instead of on every
    // append, so the log is not flooded. A retry batch is exempt: its legs mirror sub-system fills
    // that already happened, so they are merged into the new batch instead (the prices are re-set
    // at the next open).
    if (!m_open_pending_suggestions.empty() && !m_open_pending_retry) {
        if (!m_open_pending_warned) {
            m_open_pending_warned = true;
            HKU_WARN(
              "{}: un-executed open pending suggestions ({} from {}) were never consumed, "
              "dropped and replaced by the latest batch!",
              name(), m_open_pending_suggestions.size(), m_open_pending_date);
        }
        m_open_pending_suggestions.clear();
    }
    m_open_pending_date = date;
    if (!m_open_pending_suggestions.empty() && m_open_pending_retry) {
        m_open_pending_adjust = m_open_pending_adjust || is_adjust;
        TradeSuggestionList merged = m_open_pending_suggestions;
        merged.insert(merged.end(), suggestions.begin(), suggestions.end());
        m_open_pending_suggestions = merged;
    } else {
        m_open_pending_suggestions = suggestions;
        m_open_pending_adjust = is_adjust;
    }
}

bool MultiSystem::_isAdjustDate(const Datetime& date) const {
    // The external rebalancing day table takes precedence: the rebalancing is only performed on the
    // dates hitting the table (used when PF maps the master adjust_mode / delay_to_trading_day)
    if (!date.isNull()) {
        if (!m_adjust_dates.empty()) {
            return m_adjust_dates.find(date.startOfDay()) != m_adjust_dates.end();
        }
        // The calendar modes use the period rule directly (the bulk run and the live / nested
        // entries share it, so a single-bar live feed is handled correctly)
        if (!_periodStart(date).isNull()) {
            return _isAdjustDateByPeriod(date, m_last_adjust_period);
        }
    }
    // Fallback: rebalance every m_adjust_cycle close days
    if (m_adjust_cycle <= 1) {
        return true;
    }
    return (m_close_day_index % static_cast<size_t>(m_adjust_cycle)) == 0;
}

bool MultiSystem::_isAdjustDateByPeriod(const Datetime& date, const Datetime& last_period) const {
    Datetime period_start = _periodStart(date);
    HKU_IF_RETURN(period_start.isNull(), false);
    Datetime target = period_start + Days(m_adjust_cycle > 0 ? m_adjust_cycle - 1 : 0);
    if (!m_delay_to_trading_day) {
        return date.startOfDay() == target;
    }
    return date.startOfDay() >= target && last_period != period_start;
}

TradeSuggestionList MultiSystem::_netSuggestions(const Datetime& date,
                                                 const TradeSuggestionList& suggestions) const {
    struct Leg {
        double buy{0.0};
        double sell{0.0};
        double clear_num{0.0};
        bool clear{false};
        price_t buy_price{0.0};
        price_t sell_price{0.0};
        price_t clear_price{0.0};
    };
    std::map<std::pair<Stock, bool>, Leg> legs;
    for (const auto& s : suggestions) {
        if (s.stock.isNull() || s.plan_price <= 0.0) {
            continue;
        }
        if (s.type != SuggestionType::BUY && s.type != SuggestionType::SELL &&
            s.type != SuggestionType::CLEAR) {
            continue;
        }
        Leg& leg = legs[std::make_pair(s.stock, s.short_side)];
        double num = std::abs(s.number);
        if (s.type == SuggestionType::BUY) {
            leg.buy += num;
            leg.buy_price = s.plan_price;
        } else if (s.type == SuggestionType::SELL) {
            leg.sell += num;
            leg.sell_price = s.plan_price;
        } else {
            leg.clear = true;
            leg.clear_num += num;
            leg.clear_price = s.plan_price;
        }
    }

    TradeSuggestionList result;
    HKU_IF_RETURN(!m_tm, result);
    for (const auto& item : legs) {
        const Stock& stock = item.first.first;
        const bool short_side = item.first.second;
        const Leg& leg = item.second;
        double current =
          short_side ? m_tm->getShortPosition(stock).number : m_tm->getPosition(date, stock).number;
        // The same-batch legs that increase the side: the CLEAR may cover them when it is the
        // full-close form (number >= MAX_DOUBLE)
        double same_batch = short_side ? leg.sell : leg.buy;
        double clear_amount = 0.0;
        if (leg.clear) {
            // A CLEAR only clears the attributable part (the sum of the clearing legs), never the
            // whole parent book when several child books hold the same instrument
            double cap = (leg.clear_num <= 0.0 || leg.clear_num >= MAX_DOUBLE)
                           ? (current + same_batch)
                           : leg.clear_num;
            clear_amount = std::min(std::max(0.0, cap), current + same_batch);
        }
        // net > 0: buy a long / cover a short; net < 0: sell a long / open a short
        double net =
          short_side ? (clear_amount + leg.buy - leg.sell) : (leg.buy - leg.sell - clear_amount);
        if (std::abs(net) < 1e-9) {
            continue;
        }
        price_t price = net > 0.0 ? (leg.buy > 0.0 ? leg.buy_price : leg.clear_price)
                                  : (leg.sell > 0.0 ? leg.sell_price : leg.clear_price);
        if (price <= 0.0) {
            continue;
        }
        TradeSuggestion s;
        s.stock = stock;
        s.short_side = short_side;
        s.type = net > 0.0 ? SuggestionType::BUY : SuggestionType::SELL;
        s.number = std::abs(net);
        s.plan_price = price;
        result.push_back(s);
    }
    return result;
}

void MultiSystem::_executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                                      KQuery::KType ktype, TradeRecordList& out_trades) {
    TradeSuggestionList netted = _netSuggestions(date, suggestions);
    // The cash-generating group runs first (sell a long, open a short) so that the freed cash backs
    // the cash-consuming group (cover a short, buy a long) that runs afterwards
    for (const auto& s : netted) {
        if (s.stock.isNull()) {
            continue;
        }
        // The netted list only carries BUY / SELL: the buys and the short covers belong to the
        // cash-consuming group below
        if (s.type != SuggestionType::SELL) {
            continue;
        }
        // SELL: sell the suggested quantity (in mode A it is rewritten to the parent account
        // quantity; in mode B the sub-instruction is passed through). The number is always
        // positive; a short-side SELL opens/increases a short (sellShort)
        double num = s.number;
        if (num < 0.0) {
            num = -num;  // Defensive: legacy custom AF may still emit a negative number
        }
        if (num <= 0.0) {
            continue;
        }
        if (s.short_side && s.type == SuggestionType::SELL) {
            // A netted short open must still respect the lot size
            double min_trade = s.stock.minTradeNumber();
            if (min_trade > 0.0) {
                num = std::floor(num / min_trade) * min_trade;
                if (num < min_trade) {
                    continue;
                }
            }
        }
        // No quote (e.g. suspended): a 0-price sell would wipe the holding, skip it
        if (s.plan_price <= 0.0) {
            HKU_WARN("Skip the SELL suggestion, {} has no quote on {}!", s.stock.market_code(),
                     date);
            continue;
        }
        TradeRecord tr = s.short_side ? m_tm->sellShort(date, s.stock, s.plan_price, num, 0.0, 0.0,
                                                        s.plan_price, PART_SYSTEM, "MultiSystem")
                                      : m_tm->sell(date, s.stock, s.plan_price, num, 0.0, 0.0,
                                                   s.plan_price, PART_SYSTEM, "MultiSystem");
        if (!tr.isNull()) {
            out_trades.push_back(tr);
        }
    }

    // L3 cash feasibility: the mandatory short covers claim the cash first (a short left open is
    // not the intended target and a cover reduces the wrong-direction exposure); only the long
    // buys are scaled down, with the cash left after the covers as their budget. The covers need
    // full cash (buyShort has no financing branch), the long buys follow the TradeManager buying
    // power: cash / margin rate with automatic cash financing, otherwise 1:1. The requirement
    // always includes the commission, otherwise the last leg of a topped-up batch is rejected
    const bool margin_trading =
      m_tm->haveParam("support_borrow_cash") && m_tm->getParam<bool>("support_borrow_cash");
    auto buy_cost_total = [&](const TradeSuggestion& s, double qty) {
        return qty * s.plan_price * s.stock.unit() +
               m_tm->getBuyCost(date, s.stock, s.plan_price, qty).total;
    };
    double need_cover_cash = 0.0;
    double need_buy_cash = 0.0;  // the scale denominator: the margin when financing, full otherwise
    double need_buy_full = 0.0;  // the full-cash requirement, used by the auto_checkin top-up
    for (const auto& s : netted) {
        if (s.stock.isNull() || s.plan_price <= 0.0) {
            continue;
        }
        if (s.type != SuggestionType::BUY) {
            continue;  // SELL legs generate cash
        }
        double min_trade = s.stock.minTradeNumber();
        if (min_trade <= 0.0) {
            continue;
        }
        double qty = std::floor(s.number / min_trade) * min_trade;
        if (qty < min_trade) {
            continue;
        }
        if (s.short_side) {
            need_cover_cash += buy_cost_total(s, qty);
        } else {
            need_buy_full += buy_cost_total(s, qty);
            if (margin_trading) {
                double rate = m_tm->getMarginRate(date, s.stock);
                need_buy_cash += buy_cost_total(s, qty) * ((rate > 0.0) ? rate : 1.0);
            } else {
                need_buy_cash += buy_cost_total(s, qty);
            }
        }
    }
    price_t cash = m_tm->getFunds(date, ktype).cash;
    // The optional top-up (aligned with MoneyManagerBase::auto_checkin): deposit the FULL gap (the
    // covers plus the unfinanced long buys), so every leg is cash-covered and can be filled at its
    // planned quantity without borrowing. It raises the parent principal instead of borrowing, so
    // it is meant to keep the planned quantities tradable, not as a funding policy
    if (m_auto_checkin && need_cover_cash + need_buy_full > cash) {
        price_t gap = roundUp(need_cover_cash + need_buy_full - cash, m_tm->precision());
        if (m_tm->checkin(date, gap)) {
            cash = m_tm->getFunds(date, ktype).cash;
        } else {
            HKU_WARN(
              "{}: auto_checkin could not deposit the cash gap, the batch is scaled to the current "
              "cash!",
              name());
        }
    }
    double cash_scale = 1.0;
    if (need_buy_cash > 0.0) {
        double available = cash - need_cover_cash;
        if (available < need_buy_cash) {
            cash_scale = available > 0.0 ? available / need_buy_cash : 0.0;
            HKU_WARN(
              "The BUY suggestions need {:.2f} cash but only {:.2f} left after the covers, scale "
              "down by {:.2f}!",
              need_buy_cash, available, cash_scale);
        }
    }

    // The short covers: a netted cover buys back the targeted quantity (capped at the current
    // short book)
    for (const auto& s : netted) {
        if (s.stock.isNull() || !s.short_side || s.type != SuggestionType::BUY) {
            continue;
        }
        if (s.plan_price <= 0.0) {
            HKU_WARN("Skip the BUY suggestion, {} has no quote on {}!", s.stock.market_code(),
                     date);
            continue;
        }
        double short_num = m_tm->getShortPosition(s.stock).number;
        if (short_num <= 0.0) {
            continue;
        }
        double min_trade = s.stock.minTradeNumber();
        double target = std::min(s.number, short_num);
        double num = min_trade > 0.0 ? std::floor(target / min_trade) * min_trade : target;
        if (num < min_trade) {
            num = 0.0;  // Below one lot: the exchange refuses such an order
        }
        if (num <= 0.0) {
            continue;
        }
        TradeRecord tr = m_tm->buyShort(date, s.stock, s.plan_price, num, 0.0, 0.0, s.plan_price,
                                        PART_SYSTEM, "MultiSystem");
        if (!tr.isNull()) {
            out_trades.push_back(tr);
            cash = m_tm->getFunds(date, ktype).cash;
        }
    }

    for (const auto& s : netted) {
        if (s.stock.isNull() || s.type != SuggestionType::BUY || s.short_side) {
            continue;  // The short covers have already run above
        }
        if (s.number <= 0.0) {
            continue;
        }
        if (s.plan_price <= 0.0) {
            HKU_WARN("Skip the BUY suggestion, {} has no quote on {}!", s.stock.market_code(),
                     date);
            continue;
        }
        double min_trade = s.stock.minTradeNumber();
        if (min_trade <= 0.0) {
            HKU_WARN("Invalid minTradeNumber {} of {}, skip the BUY suggestion!", min_trade,
                     s.stock.market_code());
            continue;
        }
        double qty = std::floor(s.number * cash_scale / min_trade) * min_trade;
        if (margin_trading && qty >= min_trade) {
            // The TradeManager finances an order only when it exceeds the remaining cash, and a
            // cash-covered order consumes the cash 1:1: the buying power must be re-evaluated here
            // leg by leg, otherwise the tail of the batch is rejected by the TM
            constexpr double BUY_POWER_MARGIN = 0.995;
            double rate = m_tm->getMarginRate(date, s.stock);
            double power = (rate > 0.0 ? cash / rate : cash) * BUY_POWER_MARGIN;
            double unit_price = s.plan_price * s.stock.unit();
            double cap_qty =
              (unit_price > 0.0) ? std::floor(power / unit_price / min_trade) * min_trade : 0.0;
            if (cap_qty < qty) {
                HKU_WARN(
                  "The BUY leg of {} is reduced from {:.0f} to {:.0f} by the financing buying "
                  "power!",
                  s.stock.market_code(), qty, cap_qty);
                qty = cap_qty;
            }
        }
        if (qty >= min_trade) {
            TradeRecord tr = m_tm->buy(date, s.stock, s.plan_price, qty, 0.0, 0.0, s.plan_price,
                                       PART_SYSTEM, "MultiSystem");
            if (!tr.isNull()) {
                out_trades.push_back(tr);
                cash = m_tm->getFunds(date, ktype).cash;
            }
        }
    }
}

MomentResult MultiSystem::runMoment(const Datetime& datetime) {
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
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

    // The aggregate target orders queued for the next open (trade_on_close=false)
    result.pendingOnNextOpen = m_open_pending_suggestions;
    result.suggestions = m_last_suggestions;

    result.funds = m_tm->getFunds(datetime, ktype);

    // Mirror the base composite contract: surface the parent account's current holdings so that an
    // upper aggregate or the live driver sees the aggregate positions rather than an empty list
    PositionRecordList positions = m_tm->getPositionList();
    result.positions.insert(result.positions.end(), positions.begin(), positions.end());
    PositionRecordList short_positions = m_tm->getShortPositionList();
    result.positions.insert(result.positions.end(), short_positions.begin(), short_positions.end());
    return result;
}

MomentResult MultiSystem::runMomentOnOpen(const Datetime& datetime) {
    if (m_sub_index.empty() && !m_not_ready_warned) {
        m_not_ready_warned = true;
        HKU_WARN("[{}] driven without readyForRun: no sub-system is indexed, nothing will run!",
                 name());
    }
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
    result.datetime = datetime;
    // The parent handles the delisted instruments uniformly at the open stage: force selling the
    // parent holdings
    TradeRecordList delist_trades = _forceSellDelisted(datetime);
    for (auto& tr : delist_trades) {
        result.tradesOnOpen.push_back(tr);
        m_trade_list.push_back(tr);
    }

    // The deferred quota calibration of the last rebalancing close executes at this open, before
    // the queued suggestions, so all legs share one price basis
    TradeRecordList calibration_trades;
    Datetime calibration_date = m_pending_calibration_date;
    double calibration_assets = m_pending_calibration_assets;
    bool had_calibration = !m_pending_calibration.empty();
    if (had_calibration) {
        _executePendingCalibration(datetime, m_kdata.getQuery().kType(), calibration_trades);
        for (auto& tr : calibration_trades) {
            result.tradesOnOpen.push_back(tr);
            m_trade_list.push_back(tr);
        }
    }

    // trade_on_close=false: the suggestions converted on the last rebalancing day are executed
    // uniformly at this open (re-priced by the open price of the day)
    TradeRecordList executed_open;
    Datetime pending_date = m_open_pending_date;
    double pending_assets = m_open_pending_assets;
    if (!m_trade_on_close && !m_open_pending_suggestions.empty()) {
        TradeSuggestionList executable;
        TradeSuggestionList retry;
        for (auto& s : m_open_pending_suggestions) {
            price_t op = _getOpenPrice(datetime, s.stock);
            if (op > 0.0) {
                s.plan_price = op;
                executable.push_back(s);
            } else {
                retry.push_back(s);  // No quote (suspension): leave it to the next open
            }
        }
        KQuery::KType kt = m_kdata.getQuery().kType();
        if (!executable.empty()) {
            _executeSuggestions(datetime, executable, kt, executed_open);
            for (auto& tr : executed_open) {
                result.tradesOnOpen.push_back(tr);
                m_trade_list.push_back(tr);
            }
        }
        m_open_pending_suggestions = retry;
        m_open_pending_retry = !retry.empty();
        if (retry.empty()) {
            m_open_pending_date = Datetime();
            m_open_pending_assets = 0.0;
        }
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
            if (_subTreeHoldNumber(*iter, datetime) <= 0.0) {
                iter = m_force_sell_list.erase(iter);
                continue;
            }
            if (m_sell_at_not_selected) {
                TradeRecordList sold;
                _forceSellSubTree(*iter, datetime, true, sold);
                for (auto& tr : sold) {
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

    // The turnover of the rebalancing executions at this open: the deferred calibration legs (a
    // toc=false rebalancing or a toc=true suspended close), a batch queued on a rebalancing close
    // and the mode C fills (a continuous-mirror batch of the modes A/B is not rebalancing turnover
    // and is not counted)
    if (!m_trade_on_close || had_calibration) {
        double traded = 0.0;
        for (const auto& tr : result.tradesOnOpen) {
            traded += tr.number * tr.realPrice * tr.stock.unit();
        }
        Datetime adjust_date = calibration_date;
        double assets = calibration_assets;
        if (adjust_date.isNull()) {
            if (m_open_pending_adjust) {
                adjust_date = pending_date;
                assets = pending_assets;
            } else if (getMode() == "C") {
                // The mode C surfaced fills keep the last rebalance attribution; a mirror-only
                // batch of the modes A / B is not rebalancing turnover and is not recorded
                adjust_date = m_open_turnover_date;
                assets = m_open_turnover_assets;
            }
        }
        if (!adjust_date.isNull() && assets > 0.0 && traded > 0.0) {
            m_adjust_turnover.push_back({adjust_date, traded / assets});
        }
        m_open_turnover_date = Datetime();
        m_open_turnover_assets = 0.0;
        if (m_open_pending_suggestions.empty()) {
            m_open_pending_adjust = false;
        }
    }
    return result;
}

MomentResult MultiSystem::runMomentOnClose(const Datetime& datetime) {
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
    result.datetime = datetime;
    KQuery::KType ktype = m_kdata.getQuery().kType();
    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    TradeRecordList executed = _closePhase(datetime);
    for (auto& tr : executed) {
        result.tradesOnClose.push_back(tr);
        m_trade_list.push_back(tr);
    }

    // The aggregate target orders queued for the next open (trade_on_close=false)
    result.pendingOnNextOpen = m_open_pending_suggestions;
    result.suggestions = m_last_suggestions;

    result.funds = m_tm->getFunds(datetime, ktype);
    return result;
}

TradeRecordList MultiSystem::_closePhase(const Datetime& datetime) {
    KQuery::KType ktype = m_kdata.getQuery().kType();
    TradeSuggestionList suggestions;
    SubSystemContextList contexts;

    // A calibration batch left over from an earlier close without an open in between is stale:
    // drop it (with a notice), like the queued suggestion batch
    if (!m_trade_on_close && !m_pending_calibration_retry && !m_pending_calibration.empty() &&
        !m_pending_calibration_date.isNull() &&
        m_pending_calibration_date.startOfDay() != datetime.startOfDay()) {
        HKU_WARN("{}: un-executed deferred calibration legs from {} dropped on {}!", name(),
                 m_pending_calibration_date, datetime);
        m_pending_calibration.clear();
        m_pending_calibration_date = Datetime();
        m_pending_calibration_assets = 0.0;
    }

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
    // Remember the period of the calendar rule (the external table has its own schedule)
    if (is_adjust && m_adjust_dates.empty()) {
        m_last_adjust_period = _periodStart(datetime);
    }
    // Declare the rebalancing day before the sub-systems are driven: the runMoment* / intraday live
    // / nested entries have no precomputed axis, while the bulk run() entry has already declared
    // the whole run (the call is a no-op then)
    if (is_adjust) {
        _declareCycleDate(datetime);
    }
    // The total assets before any trade of this close stage: the rebalancing turnover rate is
    // measured against it (the convention is the opening capital of the period)
    const double assets_before = m_tm->getFunds(datetime, ktype).total_assets();
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

    // Mode A (Signal Aggregation): the shadows run continuously, so the parent owns its whole
    // close path (drive every sub-system + sample the selected ones) -- see _closePhaseModeA
    if (getMode() == "A") {
        return _closePhaseModeA(datetime, is_adjust, selected, se_scores, ktype, assets_before);
    }

    // Mode B without an SE: every sub-system is treated as selected (the mode A/C convention).
    // Otherwise no quota would ever be allocated and the zero-cash shadow accounts would stay
    // empty for the whole run (an empty backtest)
    const bool select_all = (getMode() == "B" && !m_se);
    if (is_adjust && select_all) {
        for (auto& sys : m_sys_list) {
            selected.insert(sys.get());
        }
    }

    // Mode B (master SimplePortfolio compatibility): on the rebalancing day the quota must be
    // allocated BEFORE driving the sub-systems, so every selected sub-system trades with its exact
    // allocated quota (the master AF adjusted the funds first and ran the systems afterwards)
    bool quota_calibrated = false;
    bool adjust_running_sys = true;
    AllocateFundsBase::Weights af_weights;
    std::unordered_map<System*, price_t> quota_map;
    // The running sub-systems for adjust_running_sys=false. Mode B keeps no running pool of its
    // own (m_running_set is mode C-only), and a mode B sub-account starts from zero and is funded
    // only by the quota injection, so a context still holding value at the calibration time was
    // funded in a previous period, i.e. it is a running sub-system
    std::set<System*> running_subs;
    if (is_adjust && getMode() == "B" && getAF()) {
        // adjust_running_sys=false: the running sub-systems keep their current holdings on the
        // rebalancing day; only the new sub-systems get the remaining parent cash (the master
        // _adjust_without_running semantics)
        adjust_running_sys = m_af->getParam<bool>("adjust_running_sys");
        SubSystemContextList quota_ctxs;
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            SubSystemContext ctx;
            ctx.sys = sys;
            auto sc = se_scores.find(sys.get());
            if (sc != se_scores.end()) {
                ctx.score = sc->second;
            }
            if (sys->getTM()) {
                ctx.funds = sys->getTM()->getFunds(datetime, ktype);
                if (ctx.funds.net_assets() > 0.0) {
                    running_subs.insert(sys.get());
                }
            }
            quota_ctxs.push_back(ctx);
        }
        if (!quota_ctxs.empty()) {
            af_weights =
              m_af->allocateQuota(datetime, m_tm, quota_ctxs, m_kdata.getQuery(), getMode());
            for (auto& ctx : quota_ctxs) {
                quota_map[ctx.sys.get()] = ctx.quota;
            }
            // The outer mode B layer may have assigned this aggregate a budget: scale the leaf
            // quotas to that total (the FOF/MOM budget semantics), capped by the inner reserve
            // (max_total_position x the inner net assets)
            if (m_external_quota_set) {
                price_t total = 0.0;
                for (const auto& kv : quota_map) {
                    total += kv.second;
                }
                price_t budget = m_external_quota;
                double fixed_amount = m_af->getParam<double>("fixed_amount");
                double max_total = m_af->getParam<double>("max_total_position");
                if (fixed_amount <= 0.0 && max_total > 0.0 && max_total < 1.0) {
                    price_t cap = max_total * m_tm->getFunds(datetime, ktype).net_assets();
                    if (budget > cap) {
                        if (!m_external_quota_capped_warned) {
                            m_external_quota_capped_warned = true;
                            HKU_WARN(
                              "The delegated quota {:.2f} exceeds the inner reserve cap "
                              "{:.2f} (max_total_position), capped!",
                              budget, cap);
                        }
                        budget = cap;
                    }
                }
                if (total > 0.0 && budget > 0.0) {
                    double scale = budget / total;
                    for (auto& kv : quota_map) {
                        kv.second *= scale;
                    }
                }
            }
            quota_calibrated = true;
        }
        m_external_quota_set = false;
        m_external_quota = 0.0;
    }

    // The injections are re-derived from this round's quotas: drop the stale targets of earlier
    // rounds (a lowered quota must not be re-inflated). This also runs when the SE selected nobody
    // (quota_calibrated is false in that case), so a stale target cannot survive a full clear.
    if (is_adjust && (m_se || select_all) && !m_trade_on_close) {
        m_pending_calibration.erase(
          std::remove_if(
            m_pending_calibration.begin(), m_pending_calibration.end(),
            [](const CalibrationAction& a) { return a.kind == CalibrationKind::INJECT; }),
          m_pending_calibration.end());
    }

    // The sub-systems liquidated by this round (zero quota): they must not be driven afterwards,
    // and their open-buffer fills must not be mirrored (the parent would buy back what the
    // liquidation just sold)
    std::set<const void*> liquidated;

    // Mode B (master SimplePortfolio compatibility): the calibration runs in the master order --
    // zero-quota liquidation -> per sub the unselected liquidation or the selected cash recycle ->
    // reduce the over quota -> inject the gap, so the freed cash is already in the parent pool
    // when the following injection runs
    if (quota_calibrated) {
        // A same-day re-drive rebuilds the round: the legs pushed by the previous drive of the same
        // close are replaced instead of duplicated (the retry legs of earlier days are kept)
        if (!m_pending_calibration_retry && !m_pending_calibration_date.isNull() &&
            m_pending_calibration_date.startOfDay() == datetime.startOfDay()) {
            m_pending_calibration.clear();
            m_pending_calibration_date = Datetime();
            m_pending_calibration_assets = 0.0;
        }
        // A selected sub-system with a zero quota is cleared like an unselected one: both books
        // liquidated, the sub not driven any more this close
        if (m_sell_at_not_selected) {
            for (auto& sys : m_sys_list) {
                if (selected.count(sys.get()) == 0) {
                    continue;
                }
                auto qit = quota_map.find(sys.get());
                if (qit != quota_map.end() && qit->second <= 0.0) {
                    if (!adjust_running_sys && running_subs.count(sys.get()) > 0) {
                        continue;  // the running sub-system is left untouched
                    }
                    _liquidateParent(sys, datetime, executed);
                    _clearSubSystem(sys, datetime, ktype);
                    liquidated.insert(sys.get());
                }
            }
        }

        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                // The unselected: with sell_at_not_selected the parent liquidates its holding and
                // the shadow account is cleared together, so the two books stay matched; without
                // it both books keep running and the sub-system only skips this round of quota
                // calibration. have() only reports the long book, so the short book needs its own
                // check -- otherwise a parent short on an unselected instrument would never be
                // covered.
                if (m_sell_at_not_selected) {
                    // Liquidate the parent holdings and clear the shadow account together: the two
                    // books must stay matched (a composite covers all of its leaf instruments)
                    _liquidateParent(sys, datetime, executed);
                    _clearSubSystem(sys, datetime, ktype);
                }
            } else if (liquidated.count(sys.get()) == 0) {
                // The selected: recycle the sub cash first (bookkeeping). With
                // adjust_running_sys=false the running sub-system is left untouched (its idle
                // cash stays in the sub account); only a new sub-system goes through here.
                if (!adjust_running_sys && running_subs.count(sys.get()) > 0) {
                    continue;
                }
                if (m_trade_on_close) {
                    TMPtr sub_tm = sys->getTM();
                    if (sub_tm) {
                        FundsRecord funds = sub_tm->getFunds(datetime, ktype);
                        if (funds.cash > 0.0) {
                            sub_tm->checkout(datetime, funds.cash);
                        }
                    }
                }
                // With trade_on_close=false the recycle is skipped: the injection of this round
                // lands at this open (or at a later open while a sell leg still waits for a
                // quote), so the idle cash stays in the sub ledger and the gap injection accounts
                // for it
            }
        }
        // The reduction of all the over quota sub-systems runs before the injection (the master
        // AF order), so the freed cash is available to the injection phase
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            if (!adjust_running_sys && running_subs.count(sys.get()) > 0) {
                continue;  // adjust_running_sys=false: the running sub-system is not force-reduced
            }
            auto qit = quota_map.find(sys.get());
            if (qit != quota_map.end() && liquidated.count(sys.get()) == 0) {
                _reduceSubSystemToQuota(sys, datetime, qit->second, ktype, executed);
            }
        }
        // The cash already freed by the reduction phase, shared by the injections: the total must
        // never exceed what the parent can actually fund
        price_t cash_left = m_tm->getFunds(datetime, ktype).cash;
        for (size_t i = 0; i < m_sys_list.size(); ++i) {
            SystemPtr& sys = m_sys_list[i];
            if (selected.count(sys.get()) == 0) {
                continue;
            }
            if (!adjust_running_sys && running_subs.count(sys.get()) > 0) {
                continue;  // adjust_running_sys=false: the running sub-system is not force-injected
            }
            auto qit = quota_map.find(sys.get());
            if (qit != quota_map.end() && liquidated.count(sys.get()) == 0) {
                if (m_trade_on_close) {
                    if (cash_left <= 0.0) {
                        continue;
                    }
                    cash_left -= _injectSubSystemGap(sys, datetime, qit->second, ktype, cash_left);
                } else {
                    // Defer the injection to the next open: the actual cash freed by the deferred
                    // sells is known then (no expectation mismatch on a gap)
                    m_pending_calibration.push_back(
                      {CalibrationKind::INJECT, sys, Stock(), 0.0, qit->second});
                }
            }
        }
    }

    if (mode_c) {
        _closePhaseModeC(datetime, is_adjust, se_selected, suggestions, executed);
        // Mode C: nothing is accumulated as pending and the parent does not order again (the trades
        // have already landed on the shared real account). The translated suggestions are still
        // kept in m_last_suggestions, so that this system can serve as a sub-system of an upper
        // mode A/B aggregate (the nesting translation of the design 9.3); the ratio denominator is
        // the shared account funds taken from m_sub_funds_before.
        m_last_suggestions = suggestions;
        _finishClosePhase(datetime, suggestions, executed, is_adjust, assets_before);
        return executed;
    }

    double parent_total_assets = m_tm->getFunds(datetime, ktype).total_assets();
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr sys = m_sys_list[i];
        const bool is_selected = (selected.count(sys.get()) > 0);
        if (m_se && is_adjust && (!is_selected || liquidated.count(sys.get()) > 0)) {
            // The unselected sub-system: it has already been liquidated (executed immediately) when
            // the quota was calibrated; without a quota pre-calculation (e.g. no AF) the
            // liquidation suggestion is generated here
            if (m_sell_at_not_selected) {
                if (!quota_calibrated) {
                    std::set<Stock> stocks;
                    const Stock& own = sys->getStock();
                    if (own.isNull()) {
                        _collectLeafStocks(sys, stocks);
                    } else {
                        stocks.insert(own);
                    }
                    TMPtr own_tm = sys->getTM();
                    const bool has_own_book = own_tm && own_tm != m_tm;
                    for (const auto& stock : stocks) {
                        price_t plan_price = _getClosePrice(datetime, stock);
                        double long_num = m_tm->getPosition(datetime, stock).number;
                        double short_num = m_tm->getShortPosition(stock).number;
                        if (has_own_book) {
                            long_num =
                              std::min(long_num, own_tm->getPosition(datetime, stock).number);
                            short_num = std::min(short_num, own_tm->getShortPosition(stock).number);
                        }
                        if (long_num > 0.0) {
                            TradeSuggestion s;
                            s.stock = stock;
                            s.sys = sys;
                            s.type = SuggestionType::CLEAR;
                            s.plan_price = plan_price;
                            s.number = long_num;
                            suggestions.push_back(s);
                        }
                        // The parent short on the unselected instrument: a short-side CLEAR covers
                        // it (a plain CLEAR only touches the long book)
                        if (short_num > 0.0) {
                            TradeSuggestion s;
                            s.stock = stock;
                            s.sys = sys;
                            s.type = SuggestionType::CLEAR;
                            s.short_side = true;
                            s.plan_price = plan_price;
                            s.number = short_num;
                            suggestions.push_back(s);
                        }
                    }
                    // Clear the sub-system shadow account as well (the master AF liquidated the sub
                    // account and recycled all its funds). Gated by sell_at_not_selected together
                    // with the parent liquidation above: in mode B the two books must stay matched
                    _clearSubSystem(sys, datetime, ktype);
                }
                continue;
            }

            // Without sell_at_not_selected the shadow keeps running and is not liquidated. Its
            // fills already executed at the open of this day must still be mirrored, otherwise the
            // two books would diverge; the sub-system itself is not driven on this rebalance day
            TradeRecordList open_trades = m_open_trades[i];
            m_open_trades[i].clear();
            if (!open_trades.empty()) {
                TradeSuggestionList subsug =
                  _toSuggestions(sys, open_trades, m_sub_funds_before[i], datetime);
                for (auto& s : subsug) {
                    suggestions.push_back(s);
                }
            }
            continue;
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
        if (sys->getTM()) {
            ctx.funds = sys->getTM()->getFunds(datetime, ktype);
        }
        // Backfill the SE score (0 on the non-rebalancing days / for the unselected ones), used
        // by AF_MultiFactor etc. to take the scores as the weights
        auto score_it = se_scores.find(sys.get());
        if (score_it != se_scores.end()) {
            ctx.score = score_it->second;
        }
        if (parent_total_assets > 0.0) {
            ctx.current_weight = ctx.funds.total_assets() / parent_total_assets;
        }
        // total_return stays 0 here: it is the mode A shadow return over the fixed signal cash, and
        // a mode B sub account starts from zero (no fixed initial cash)
        contexts.push_back(ctx);
    }

    if (!is_adjust) {
        // Continuous mirror (mode B): the parent mirrors the sub-system trades of this bar at once
        // (mode A returns earlier and mode C trades on the shared account, so only mode B reaches
        // here); the quota calibration stays on the rebalancing day only. No L1 runs here, so the
        // L2 pass-through is called with an empty weight table: the built-in mode B _toTargets
        // ignores the weights by contract (it passes the sub-system instruction through unchanged).
        // A custom AF that overrides _toTargets and needs the weights must run its own L1 here.
        if (!suggestions.empty()) {
            getAF()->allocateTargets(datetime, m_tm, suggestions, AllocateFundsBase::Weights(),
                                     m_kdata.getQuery(), getMode());
            if (m_trade_on_close) {
                _executeSuggestions(datetime, suggestions, ktype, executed);
            } else {
                _queueOpenPending(datetime, suggestions, is_adjust);
            }
        }
        m_last_suggestions = suggestions;
        _finishClosePhase(datetime, suggestions, executed, is_adjust, assets_before);
        return executed;
    }

    // Only mode B reaches here: mode A has its own close path (_closePhaseModeA) and mode C trades
    // directly on the shared account, both returning before this point, so the mode guards below
    // are dropped.
    if (quota_calibrated) {
        // The quota has been calibrated into the sub-system shadow accounts before driving
        // (the master order: adjust the funds first, run the systems afterwards). Here only
        // the L2 pass-through and the L3 risk control remain, then the parent orders uniformly.
        getAF()->allocateTargets(datetime, m_tm, suggestions, af_weights, m_kdata.getQuery(),
                                 getMode());
        if (!suggestions.empty()) {
            if (m_trade_on_close) {
                _executeSuggestions(datetime, suggestions, ktype, executed);
            } else {
                // trade_on_close=false: convert first, execute uniformly at the next open
                _queueOpenPending(datetime, suggestions, is_adjust);
            }
        }
    } else {
        // No quota pre-calculation (no AF, or the SE selected nobody): keep the suggestion-driven
        // legacy flow, L1 runs here and the next-period quota is written back on the rebalancing
        // day (lagging one period behind, quota penetration).
        getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery(), getMode());
        // A zero quota takes the sub-system out of the allocation: liquidate both books and drop
        // its same-close fills from the mirror batch (the sub ledger is cleared as a whole, so
        // mirroring those fills would leave the parent with a phantom position)
        if (m_sell_at_not_selected) {
            const bool adjust_running_sys = m_af->getParam<bool>("adjust_running_sys");
            for (auto& ctx : contexts) {
                if (ctx.quota > 0.0) {
                    continue;
                }
                if (!adjust_running_sys && ctx.sys->getTM() &&
                    ctx.sys->getTM()->getFunds(datetime, ktype).net_assets() > 0.0) {
                    continue;  // the running sub-system is left untouched
                }
                suggestions.erase(
                  std::remove_if(suggestions.begin(), suggestions.end(),
                                 [&](const TradeSuggestion& s) { return s.sys == ctx.sys; }),
                  suggestions.end());
                _liquidateParent(ctx.sys, datetime, executed);
                _clearSubSystem(ctx.sys, datetime, ktype);
            }
        }
        if (!suggestions.empty()) {
            if (m_trade_on_close) {
                _executeSuggestions(datetime, suggestions, ktype, executed);
            } else {
                // trade_on_close=false: convert first, execute uniformly at the next open
                _queueOpenPending(datetime, suggestions, is_adjust);
            }
        }
        for (auto& ctx : contexts) {
            if (ctx.quota > 0.0) {
                setSubSystemQuota(ctx.sys, datetime, ctx.quota);
            }
        }
    }

    m_last_suggestions = suggestions;
    _finishClosePhase(datetime, suggestions, executed, is_adjust, assets_before);
    return executed;
}

TradeRecordList MultiSystem::_closePhaseModeA(const Datetime& datetime, bool is_adjust,
                                              const std::set<System*>& selected,
                                              const std::unordered_map<System*, double>& se_scores,
                                              KQuery::KType ktype, double assets_before) {
    TradeSuggestionList suggestions;
    TradeRecordList executed;

    // The shadows run continuously: every sub-system is driven at the close of every bar (the SE
    // selection gates the parent sampling only, it never changes the sub-system itself)
    std::vector<MomentResult> sub_results(m_sys_list.size());
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr& sys = m_sys_list[i];
        sub_results[i] = sys->runMomentOnClose(datetime);
    }

    if (!is_adjust) {
        // Non-rebalancing day: the parent does nothing (it holds the previous target)
        m_last_suggestions.clear();
        _finishClosePhase(datetime, suggestions, executed, is_adjust, assets_before);
        return executed;
    }
    m_last_sampled_date = datetime;

    // Sampling point: build the L1 context of every selected sub-system and turn its current net
    // exposure snapshot into the per-instrument net target suggestions for the L2 synthesis
    const double parent_total_assets = m_tm->getFunds(datetime, ktype).total_assets();
    SubSystemContextList contexts;
    for (size_t i = 0; i < m_sys_list.size(); ++i) {
        SystemPtr& sys = m_sys_list[i];
        if (m_se && selected.count(sys.get()) == 0) {
            continue;  // Unselected: zero signal, its instruments drop out of the parent target
        }
        SubSystemContext ctx;
        ctx.sys = sys;
        if (sys->getTM()) {
            ctx.funds = sys->getTM()->getFunds(datetime, ktype);
        }
        // The L1 context carries the scale of the sub account over the parent real account and the
        // shadow cumulative return over the initial signal cash (mode A only: the mode B shadow
        // account starts from zero and has no fixed initial cash)
        if (parent_total_assets > 0.0) {
            ctx.current_weight = ctx.funds.total_assets() / parent_total_assets;
        }
        if (m_sub_init_cash > 0.0) {
            ctx.total_return = (ctx.funds.total_assets() - m_sub_init_cash) / m_sub_init_cash;
        }
        auto sc = se_scores.find(sys.get());
        if (sc != se_scores.end()) {
            ctx.score = sc->second;
        }
        contexts.push_back(ctx);
        _fillNetRatioSuggestions(sys, sub_results[i], datetime, suggestions);
    }

    // Include the parent's current holdings in the target set: an orphan position (held but no
    // longer covered by any sub-system) is flattened to zero instead of lingering forever. Only
    // the instruments of this aggregate's own sub-systems are in scope: under a shared parent
    // account (mode C) the positions of the other children must not be treated as orphans
    {
        std::set<Stock> covered;
        for (const auto& s : suggestions) {
            covered.insert(s.stock);
        }
        std::set<Stock> own_stocks;
        for (const auto& sys : m_sys_list) {
            _collectLeafStocks(sys, own_stocks);
        }
        std::vector<PositionRecord> zero_targets;
        for (const auto& stk : own_stocks) {
            PositionRecord pos = m_tm->getPosition(datetime, stk);
            if (pos.number > 0.0) {
                zero_targets.push_back(pos);
            }
            PositionRecord short_pos = m_tm->getShortPosition(stk);
            if (short_pos.number > 0.0) {
                zero_targets.push_back(short_pos);
            }
        }
        auto add_zero_target = [&](const PositionRecord& pos) {
            if (pos.stock.isNull() || covered.count(pos.stock) > 0) {
                return;
            }
            price_t p = _getClosePrice(datetime, pos.stock);
            if (p <= 0.0) {
                return;
            }
            covered.insert(pos.stock);
            TradeSuggestion s;
            s.stock = pos.stock;
            s.from = PART_PORTFOLIO;
            s.type = SuggestionType::HOLD;
            s.plan_price = p;
            s.target_position_ratio = 0.0;
            suggestions.push_back(s);
        };
        for (const auto& pos : zero_targets) {
            add_zero_target(pos);
        }
    }

    getAF()->allocate(datetime, m_tm, suggestions, contexts, m_kdata.getQuery(), getMode());
    if (!suggestions.empty()) {
        if (m_trade_on_close) {
            _executeSuggestions(datetime, suggestions, ktype, executed);
        } else {
            _queueOpenPending(datetime, suggestions, is_adjust);
        }
    }

    m_last_suggestions = suggestions;
    _finishClosePhase(datetime, suggestions, executed, is_adjust, assets_before);
    return executed;
}

void MultiSystem::_foldSubtreePendingRequests(const SystemPtr& sys, const Datetime& date,
                                              double base, std::map<Stock, double>& ratio) {
    HKU_IF_RETURN(!sys || base <= 0.0, void());
    // A nested mode A has already folded the same leaf intents into its sampling / queued batch
    // on the day it sampled: folding them again would double count the intent
    if (auto* ms = dynamic_cast<MultiSystem*>(sys.get())) {
        if (ms->getMode() == "A" && ms->m_last_sampled_date == date) {
            return;
        }
    }
    for (const auto& child : sys->getSubSystemList()) {
        if (!child) {
            continue;
        }
        if (auto* ms = dynamic_cast<MultiSystem*>(child.get())) {
            const bool sampled_a = (ms->getMode() == "A" && ms->m_last_sampled_date == date);
            for (const auto& sug : ms->m_open_pending_suggestions) {
                if (sug.stock.isNull() || sug.plan_price <= 0.0 || sug.number <= 0.0) {
                    continue;
                }
                double num = sug.number;
                if (sug.type == SuggestionType::CLEAR) {
                    double held = sug.short_side ? ms->getTM()->getShortPosition(sug.stock).number
                                                 : ms->getTM()->getPosition(date, sug.stock).number;
                    num = std::min(num, held);
                }
                if (num <= 0.0) {
                    continue;
                }
                double v = num * sug.plan_price * sug.stock.unit() / base;
                if (sug.type == SuggestionType::BUY ||
                    (sug.type == SuggestionType::CLEAR && sug.short_side)) {
                    ratio[sug.stock] += v;
                } else if (sug.type == SuggestionType::SELL ||
                           (sug.type == SuggestionType::CLEAR && !sug.short_side)) {
                    ratio[sug.stock] -= v;
                }
            }
            if (sampled_a) {
                continue;  // its queued batch (folded above) already carries the leaf intents
            }
            if (ms->getMode() == "A") {
                if (!ms->m_open_pending_suggestions.empty()) {
                    continue;  // the converted batch already represents the leaf intents
                }
                // No converted batch: fall through to the raw leaf requests
            } else {
                _foldSubtreePendingRequests(child, date, base, ratio);
                continue;
            }
        }
        if (child->isComposite()) {
            _foldSubtreePendingRequests(child, date, base, ratio);
            continue;
        }
        Stock stk = child->getStock();
        if (stk.isNull()) {
            continue;
        }
        price_t p = _getClosePrice(date, stk);
        if (p <= 0.0) {
            continue;
        }
        auto fold = [&](const std::vector<TradeRequest>& reqs, double sign) {
            for (const auto& r : reqs) {
                if (!r.valid || r.number <= 0.0) {
                    continue;
                }
                ratio[stk] += sign * r.number * p * stk.unit() / base;
            }
        };
        fold(child->getBuyTradeRequestList(), 1.0);
        fold(child->getSellTradeRequestList(), -1.0);
        fold(child->getBuyShortTradeRequestList(), 1.0);
        fold(child->getSellShortTradeRequestList(), -1.0);
    }
}

void MultiSystem::_fillNetRatioSuggestions(const SystemPtr& sys, const MomentResult& sub,
                                           const Datetime& datetime, TradeSuggestionList& out) {
    TMPtr sub_tm = sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sys->name());
    double base = sub_tm->getFunds(datetime, m_kdata.getQuery().kType()).net_assets();
    if (base <= 0.0) {
        return;  // Non-positive NAV: no signal at this sampling point
    }
    auto price_of = [&](const Stock& s, price_t fallback) {
        price_t p = _getClosePrice(datetime, s);
        return p > 0.0 ? p : fallback;
    };
    std::map<Stock, double> ratio;  // instrument -> signed net exposure ratio
    for (const auto& pos : sub_tm->getPositionList()) {
        if (!pos.stock.isNull()) {
            double p = price_of(pos.stock, 0.0);
            if (p > 0.0) {
                ratio[pos.stock] += pos.number * p * pos.stock.unit() / base;
            }
        }
    }
    for (const auto& pos : sub_tm->getShortPositionList()) {
        if (!pos.stock.isNull()) {
            double p = price_of(pos.stock, 0.0);
            if (p > 0.0) {
                ratio[pos.stock] -= pos.number * p * pos.stock.unit() / base;
            }
        }
    }
    // The unfilled requests belong to the intent state: fold them in so that a pending sell is not
    // read as still held (a leaf sub-system acts on its own single instrument)
    Stock stk = sys->getStock();
    if (!stk.isNull()) {
        price_t p = price_of(stk, 0.0);
        if (p > 0.0) {
            for (const auto& req : sub.delayOnNextOpen) {
                if (!req.valid || req.number <= 0.0) {
                    continue;
                }
                double v = req.number * p * stk.unit() / base;
                if (req.business == BUSINESS_BUY || req.business == BUSINESS_BUY_SHORT) {
                    ratio[stk] += v;
                } else if (req.business == BUSINESS_SELL || req.business == BUSINESS_SELL_SHORT) {
                    ratio[stk] -= v;
                }
            }
        }
    }
    // Fold a nested aggregate child's queued (converted but unexecuted) orders into the intent
    for (const auto& sug : sub.pendingOnNextOpen) {
        if (sug.stock.isNull() || sug.plan_price <= 0.0 || sug.number <= 0.0) {
            continue;
        }
        // A CLEAR order number is capped at the current holding of its side (the queued orders
        // are already converted, but stay defensive against a MAX_DOUBLE full-close number)
        double num = sug.number;
        if (sug.type == SuggestionType::CLEAR) {
            double held = sug.short_side ? sub_tm->getShortPosition(sug.stock).number
                                         : sub_tm->getPosition(datetime, sug.stock).number;
            num = std::min(num, held);
        }
        if (num <= 0.0) {
            continue;
        }
        double v = num * sug.plan_price * sug.stock.unit() / base;
        // The signed net exposure delta (long minus short):
        //   + : buy a long / cover a short (BUY any side, or a short-side CLEAR)
        //   - : sell a long / open a short (SELL any side, or a long-side CLEAR)
        if (sug.type == SuggestionType::BUY ||
            (sug.type == SuggestionType::CLEAR && sug.short_side)) {
            ratio[sug.stock] += v;
        } else if (sug.type == SuggestionType::SELL ||
                   (sug.type == SuggestionType::CLEAR && !sug.short_side)) {
            ratio[sug.stock] -= v;
        }
    }
    if (sys->isComposite()) {
        _foldSubtreePendingRequests(sys, datetime, base, ratio);
    }
    for (auto& kv : ratio) {
        if (kv.first.isNull() || kv.second == 0.0) {
            continue;
        }
        TradeSuggestion s;
        s.stock = kv.first;
        s.sys = sys;
        s.from = PART_SYSTEM;
        s.type =
          SuggestionType::HOLD;  // A snapshot carrier, the direction is the sign of the ratio
        s.plan_price = price_of(kv.first, 0.0);
        s.target_position_ratio = std::clamp(kv.second, -1.0, 1.0);
        s.short_side = (kv.second < 0.0);
        out.push_back(s);
    }
}

void MultiSystem::_finishClosePhase(const Datetime& datetime,
                                    const TradeSuggestionList& suggestions,
                                    const TradeRecordList& executed, bool is_adjust,
                                    double assets_before) {
    // The close-day counter advances once per distinct trading day: a repeated close drive of the
    // same day (e.g. an intraday timer firing twice) must not shift the rebalancing rhythm
    if (m_last_close_date != datetime) {
        m_last_close_date = datetime;
        m_close_day_index++;
    }

    // The rebalancing close of a trade_on_close=false session records its date/assets here, so
    // the open stage can attribute the deferred calibration legs, a batch queued on this close
    // and the mode C fills to that close (a continuous-mirror batch of A/B is not rebalancing
    // turnover)
    if (is_adjust) {
        m_open_turnover_date = datetime;
        m_open_turnover_assets = assets_before;
    }

    // The deferred calibration legs (a trade_on_close=false rebalancing, or a trade_on_close=true
    // close suspended by a missing quote) record their date/assets here, so the open stage can
    // attribute the executed legs to the rebalancing day (all modes)
    if (!m_pending_calibration.empty() && m_pending_calibration_date.isNull()) {
        m_pending_calibration_date = datetime;
        m_pending_calibration_assets = assets_before;
    }

    // The rebalancing turnover rate: the turnover amount / the total assets before rebalancing
    // (recorded on the rebalancing days with actual trades; the mode C close-stage force sells
    // count even with trade_on_close=false)
    if (is_adjust && (m_trade_on_close || getMode() == "C") && !executed.empty()) {
        double turnover_cash = 0.0;
        for (auto& tr : executed) {
            turnover_cash += std::fabs(tr.realPrice) * tr.number * tr.stock.unit();
        }
        m_adjust_turnover.emplace_back(datetime,
                                       assets_before > 0.0 ? turnover_cash / assets_before : 0.0);
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
        // The adopted prototypes are the running systems themselves, so record the identity
        // mapping: a logical selector (OperatorSelector) relies on it to tell which operand side
        // each running system belongs to, otherwise every combination would select nothing.
        m_se->bindRealToProto(proto, proto);
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

void MultiSystem::_collectLeafStocks(const SystemPtr& sys, std::set<Stock>& out) {
    if (!sys) {
        return;
    }
    const Stock& stk = sys->getStock();
    if (!stk.isNull()) {
        out.insert(stk);
        return;
    }
    for (auto& sub : sys->getSubSystemList()) {
        _collectLeafStocks(sub, out);
    }
}

SystemPtr MultiSystem::_findLeafByStock(const SystemPtr& sys, const Stock& stock) {
    if (!sys) {
        return SystemPtr();
    }
    if (sys->getStock() == stock) {
        return sys;
    }
    for (auto& sub : sys->getSubSystemList()) {
        auto found = _findLeafByStock(sub, stock);
        if (found) {
            return found;
        }
    }
    return SystemPtr();
}

double MultiSystem::_subTreeHoldNumber(const SystemPtr& sys, const Datetime& date) const {
    std::set<Stock> stocks;
    _collectLeafStocks(sys, stocks);
    double total = 0.0;
    for (auto& stk : stocks) {
        total += m_tm->getHoldNumber(date, stk);
    }
    return total;
}

void MultiSystem::_forceSellSubTree(const SystemPtr& sys, const Datetime& date, bool on_open,
                                    TradeRecordList& out) {
    std::set<Stock> stocks;
    _collectLeafStocks(sys, stocks);
    for (auto& stk : stocks) {
        double num = m_tm->getHoldNumber(date, stk);
        if (num <= 0.0) {
            continue;
        }
        // A leaf trading directly on the shared account books the sale itself (the quantity is
        // sold once); a leaf sitting on the shadow account of an intermediate aggregate cannot
        // touch the real account, so the parent sells on its own account instead
        SystemPtr holder = _findLeafByStock(sys, stk);
        if (holder && holder->getTM() == m_tm) {
            TradeRecord tr = on_open ? holder->sellForceOnOpen(date, num, PART_PORTFOLIO)
                                     : holder->sellForceOnClose(date, num, PART_PORTFOLIO);
            if (!tr.isNull()) {
                out.push_back(tr);
            }
            continue;
        }
        price_t price = on_open ? _getOpenPrice(date, stk) : _getClosePrice(date, stk);
        if (price <= 0.0) {
            continue;  // No quote (suspended): the liquidation is left to the next session
        }
        TradeRecord tr =
          m_tm->sell(date, stk, price, num, 0.0, 0.0, price, PART_PORTFOLIO, "MultiSystem");
        if (!tr.isNull()) {
            out.push_back(tr);
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
    // legacy Portfolio always had an SE, this keeps the mode well-defined anyway. The cycle days
    // are declared on admission only, so without an SE the direct cycle signals never get a day.
    // A nested aggregate has no SG of its own: its cycle leaves are declared by its own mode, so
    // they must not be checked (and warned) here
    if (!m_se) {
        if (!m_no_se_cycle_warned) {
            for (auto& sys : m_sys_list) {
                auto sg = sys->getSG();
                if (sg && sg->getParam<bool>("cycle")) {
                    m_no_se_cycle_warned = true;
                    HKU_WARN(
                      "Mode C without an SE never declares cycle days, the cycle signal {} stays "
                      "silent!",
                      sg->name());
                    break;
                }
            }
        }
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
        // The internal ledgers of the removed subtree stop being driven: clear them now when the
        // forced liquidation is on (the parent shared holding is handled by the force-sell pool).
        // Without it the legacy leaves the holding unfollowed, so the child books are frozen to
        // stay consistent with the still held parent position
        if (m_sell_at_not_selected) {
            _clearSubSystemAccounts(sys, datetime, m_kdata.getQuery().kType());
        }
    }

    // The admitted ones enter the pool (in the SE order) and ONLY the new ones get the cycle start
    // declared, as the legacy Portfolio did (modes A/B declare the whole run in advance instead).
    // A pool member is never re-declared, and an SE that snapshots the buy signals (SE_Signal)
    // never admits a cycle sub-system at all
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
                sg->startCycle(datetime, _getCycleEnd(datetime));
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
        // A nested aggregate carries no stock of its own: its holding lives on the shared account
        // under the leaf instruments, so the judgement looks through the sub-tree
        if (_subTreeHoldNumber(sys, datetime) <= 0.0) {
            continue;
        }
        if (m_sell_at_not_selected) {
            _forceSellSubTree(sys, datetime, false, executed);
        } else {
            _runModeCClose(sys, datetime, suggestions, executed);
        }
        if (_subTreeHoldNumber(sys, datetime) > 0.0) {
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

KData MultiSystem::_getStockKData(const Stock& stock) {
    // The cache is dropped as soon as the driving KData advances: a live session replays with the
    // same query while the data grows every day, and a stale KData would make the price lookups
    // silently miss the new bars
    Datetime mark = m_kdata.empty() ? Datetime() : m_kdata.back().datetime;
    if (mark != m_kdata_cache_mark) {
        m_kdata_cache.clear();
        m_kdata_cache_mark = mark;
    }
    auto iter = m_kdata_cache.find(stock.market_code());
    if (iter != m_kdata_cache.end()) {
        return iter->second;
    }
    KData kdata = stock.getKData(m_kdata.getQuery());
    m_kdata_cache.emplace(stock.market_code(), kdata);
    return kdata;
}

price_t MultiSystem::_getClosePrice(const Datetime& date, const Stock& stock) {
    if (stock.isNull()) {
        return 0.0;
    }
    KData kdata = _getStockKData(stock);
    size_t pos = kdata.getPos(date);
    return pos == Null<size_t>() ? 0.0 : kdata.getKRecord(pos).closePrice;
}

price_t MultiSystem::_getOpenPrice(const Datetime& date, const Stock& stock) {
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
        // The shadow books of the sub-systems must be flattened as well
        for (auto& sys : m_sys_list) {
            _clearDelistedInSubTree(sys, pos.stock, date, price);
        }
        TradeRecord tr =
          m_tm->sell(date, pos.stock, price, MAX_DOUBLE, 0.0, 0.0, price, PART_SYSTEM, "DELIST");
        if (!tr.isNull()) {
            result.push_back(tr);
        }
    }
    // A short position in a delisted instrument is force-COVERED (buyShort), not sold
    for (auto& pos : m_tm->getShortPositionList()) {
        if (pos.stock.isNull()) {
            continue;
        }
        Datetime last_dt = pos.stock.lastDatetime();
        if (last_dt == Null<Datetime>() || last_dt >= date) {
            continue;
        }
        _removeStockFromPoolsRecursive(pos.stock);
        KData kdata = _getStockKData(pos.stock);
        if (kdata.empty()) {
            continue;
        }
        price_t price = kdata.getKRecord(kdata.size() - 1).closePrice;
        // The shadow books of the sub-systems must be flattened as well
        for (auto& sys : m_sys_list) {
            _clearDelistedInSubTree(sys, pos.stock, date, price);
        }
        TradeRecord tr = m_tm->buyShort(date, pos.stock, price, MAX_DOUBLE, 0.0, 0.0, price,
                                        PART_SYSTEM, "DELIST");
        if (!tr.isNull()) {
            result.push_back(tr);
        }
    }
    return result;
}

void MultiSystem::_clearDelistedInSubTree(const SystemPtr& sys, const Stock& stock,
                                          const Datetime& date, price_t price) {
    if (!sys) {
        return;
    }
    TMPtr tm = sys->getTM();
    if (tm && tm != m_tm) {
        if (tm->getPosition(date, stock).number > 0.0) {
            tm->sell(date, stock, price, MAX_DOUBLE, 0.0, 0.0, price, PART_PORTFOLIO, "DELIST");
        }
        if (tm->getShortPosition(stock).number > 0.0) {
            tm->buyShort(date, stock, price, MAX_DOUBLE, 0.0, 0.0, price, PART_PORTFOLIO, "DELIST");
        }
    }
    for (auto& sub : sys->getSubSystemList()) {
        _clearDelistedInSubTree(sub, stock, date, price);
    }
}

void MultiSystem::setSubSystemQuota(const SYSPtr& sub_sys, const Datetime& date, price_t quota) {
    HKU_WARN_IF_RETURN(!sub_sys, void(), "Null subsystem!");
    TMPtr sub_tm = sub_sys->getTM();
    HKU_WARN_IF_RETURN(!sub_tm, void(), "Sub system has no trade manager! {}", sub_sys->name());
    HKU_WARN_IF_RETURN(quota <= 0.0, void(), "Invalid quota {} for subsystem {}!", quota,
                       sub_sys->name());

    // Adjust the net assets of the sub-system to the target quota:
    //   - Quota increase: deposit the cash (checkin the difference)
    //   - Quota decrease: withdraw the cash (checkout the difference; when the cash is insufficient
    //   the sub-system reduces the position itself, a warning is recorded here)
    // The aggregate sub-system (nested) also triggers its internal allocation by adjusting the
    // net assets of its virtual account (quota penetration).
    KQuery::KType ktype = m_kdata.getQuery().kType();
    if (ktype.empty()) {
        ktype = KQuery::DAY;
    }
    FundsRecord funds = sub_tm->getFunds(date, ktype);
    price_t diff = quota - funds.net_assets();
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
