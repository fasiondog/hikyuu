/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  The aggregate trading system (portfolio backtesting).
 *
 *  It holds multiple sub-systems (single-security systems or nested aggregates) and drives them
 *  on the open/close stages of a fully aligned time axis, then places the orders on the single
 *  real account of the top layer. It supports arbitrary nesting (with circular reference
 *  detection) and a rebalancing cycle, and carries the portfolio-level fund allocation (AF,
 *  see AllocateFundsBase for the L1/L2/L3 layers).
 *
 *  Running modes (the mode is held by the AF, set via setMode / the AF factories):
 *
 *  - **Mode A "Signal Aggregation" (the default)**: every sub-system gets a shadow account
 *    funded with `m_sub_init_cash` as a pure signal source. On the rebalancing day the shadow
 *    signal cash is reset, so every sub-system keeps submitting its position intent; the parent
 *    converts the suggestions into the executable quantity by the L2 target conversion
 *    (weight x position ratio x the parent total assets) and orders uniformly on its own account.
 *    The shadow bookkeeping never touches the real funds.
 *
 *  - **Mode B "Fund Allocation" (quota allocation, FOF/MOM style)**: every selected sub-system is
 *    calibrated to its quota on the rebalancing day BEFORE it is driven (recycle the shadow cash,
 *    clear the unselected, reduce the over-quota part, inject the gap), so the sub-system trades
 *    with the exact allocated quota; the parent mirrors the real instructions of the sub-systems
 *    (L2 pass-through) on its own account. The shadow accounts start from zero and follow the
 *    cost function of the parent account, keeping in step with the parent trades.
 *
 *  - **Mode C "Shared Account Compatibility" (the legacy Portfolio behavior)**: no shadow account
 *    is created, every sub-system trades DIRECTLY on the single real account of the parent
 *    (`shared_tm`) and is sized by its own MM. The parent only gates the entry with the SE and
 * keeps the running set driven day by day, so that the sell signal of a sub-system is realized on
 * the very day it occurs; no L2 conversion is done and the parent does not place the order again.
 * It exists to reproduce the results of the Portfolio before the refactoring.
 *
 *  See the doc comments of AllocateFundsBase for the precise L1/L2/L3 semantics and PF_Simple /
 *  PF_WithoutAF for the preset configurations of the modes.
 */

#pragma once
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include "../System.h"
#include "../../../trade_manage/crt/crtTM.h"
#include "../../moneymanager/crt/MM_Nothing.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

class SelectorBase;  // Forward declaration, to avoid a circular include with SelectorBase.h (which
                     // includes System.h)

class HKU_API MultiSystem : public System {
public:
    MultiSystem() : System() {
        _initAxisParam();
        // The default MM of the aggregate parent system is mode A (signal aggregation, the base
        // allocate implements the equal weight allocation)
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const string& name) : System(name) {
        _initAxisParam();
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemPtr& sys) : m_sys_list(sys ? SystemList{sys} : SystemList{}) {
        _initAxisParam();
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemList& sys_list, const string& name = "MultiSystem")
    : System(name), m_sys_list(sys_list) {
        _initAxisParam();
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    virtual ~MultiSystem() = default;

    /** Add a sub-system (with the circular reference detection, it rejects the node that contains
     * itself or an existing node) */
    void add(const SystemPtr& sys);

    /** Get the sub-system list */
    const SystemList& getSystemList() const {
        return m_sys_list;
    }

    virtual bool isComposite() const override {
        return true;
    }
    virtual const SystemList& getSubSystemList() const override {
        return m_sys_list;
    }

    virtual void run(const KData& kdata, bool reset = true, bool resetAll = false) override;

    /** master compatibility overload: equivalent to master `Portfolio::run(query)`, it runs with
     * the market trading calendar as the driving axis.
     *  - Driving axis: the explicitly injected fixed time axis (axis-mode == "calendar" and not
     * empty) takes precedence, otherwise the StockManager market trading calendar
     * `get_trading_calendar(query)` is used (the SH market by default);
     *  - Context KData: it only provides the query/ktype and the price query context, its own
     * instrument `getStock()` takes precedence, then the first (recursive) sub-system instrument,
     * and finally it degenerates to the KData of the calendar benchmark index (e.g. sh000001);
     *  - The axis-mode / fixed time axis parameters are not modified, the calendar axis only drives
     * this run.
     *  @param query the query condition (also used as the query context of the sub-systems and the
     * prices)
     *  @param reset whether to reset before running (forwarded to every sub-system)
     *  @param resetAll whether to force a full reset before running
     *  @note when ktype is not the daily line, consistent with master, adjust-mode is required to
     * be query/day (the calendar axis is a daily line sequence). */
    void run(const KQuery& query, bool reset = true, bool resetAll = false);

    virtual MomentResult runMoment(const Datetime& datetime) override;
    virtual MomentResult runMomentOnOpen(const Datetime& datetime) override;
    virtual MomentResult runMomentOnClose(const Datetime& datetime) override;

    virtual void readyForRun() override;
    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual SystemPtr _clone() override;

    /** The hierarchy path */
    virtual const string& getPath() const override {
        return m_path;
    }

    /** Set the fund allocation instance (the portfolio-level fund allocation AF, carrying the three
     * algorithm parts L1/L2/L3) */
    void setAF(const AllocateFundsPtr& af) {
        if (af) {
            m_af = af;
        }
    }

    /** Get the fund allocation instance */
    const AllocateFundsPtr& getAF() const {
        return m_af;
    }

    /** Set the running mode (the mode is held by the AF, the only source):
     *  - "A" **Signal Aggregation** (the default): the sub-systems are pure signal sources on
     *    their shadow accounts, the parent converts the suggestions by the L2 target conversion
     *    and orders uniformly on its own account;
     *  - "B" **Fund Allocation** (quota allocation, FOF/MOM style): every selected sub-system is
     *    calibrated to the quota allocated by L1 on the rebalancing day and trades with the exact
     *    quota, the parent mirrors its real instructions (L2 pass-through).
     *  See the class comment and AllocateFundsBase for the full semantics. */
    void setMode(const string& mode) {
        if (m_af) {
            m_af->setMode(mode);
        }
    }

    /** Get the running mode: "A" Signal Aggregation / "B" Fund Allocation / "C" Shared Account
     *  Compatibility (from the AF) */
    const string& getMode() const;

    /** Mode C only: the running pool, i.e. the sub-systems admitted by the SE on the last
     *  rebalancing day. A sub-system leaves the pool as soon as it is no longer selected (no matter
     *  whether it still holds), which is the legacy rule of the Portfolio without AF. Runtime
     *  state: kept across bars, cleared by reset / forceResetAll, NOT rebuilt by readyForRun.
     *  Exposed for the tests and the debugging only. */
    const std::set<System*>& getRunningSet() const {
        return m_running_set;
    }

    /** Mode C only: the force-sell pool, i.e. the sub-systems that left the running pool while
     *  still holding (the legacy m_force_sell_sys_list). They are driven again only on the
     *  non-rebalancing days and only when sell_at_not_selected is on, and they leave the pool once
     *  the holding is gone. With sell_at_not_selected off their position stays unfollowed (the
     *  legacy semantics, see the design 9.3). */
    const SystemList& getForceSellList() const {
        return m_force_sell_list;
    }

    /** Set the signal cash of the sub-system shadow account, reset on every rebalancing day in
     * mode A (Signal Aggregation). In mode B (Fund Allocation) the shadow accounts start from
     * zero and the quota comes from the L1 allocation, so this value is unused. */
    void setSubInitCash(price_t cash) {
        m_sub_init_cash = cash > 0.0 ? cash : m_sub_init_cash;
    }

    /** Get the initial fund of the sub-system shadow account */
    price_t getSubInitCash() const {
        return m_sub_init_cash;
    }

    /** Set the rebalancing cycle (days); <=1 means rebalancing on every close day */
    void setAdjustCycle(int cycle) {
        m_adjust_cycle = cycle > 0 ? cycle : 1;
    }

    /** Get the rebalancing cycle (days) */
    int getAdjustCycle() const {
        return m_adjust_cycle;
    }

    /** Set whether to execute the rebalancing orders at the close stage */
    void setTradeOnClose(bool v) {
        m_trade_on_close = v;
    }

    /** Get whether to execute the rebalancing orders at the close stage */
    bool getTradeOnClose() const {
        return m_trade_on_close;
    }

    /** Set the trading object selector (optional; after it is set only the sub-systems selected by
     * SE run, the unselected ones can be liquidated) */
    void setSE(const std::shared_ptr<SelectorBase>& se) {
        m_se = se;
    }

    /** Get the trading object selector */
    const std::shared_ptr<SelectorBase>& getSE() const {
        return m_se;
    }

    /** Set whether to force liquidating the unselected sub-systems (sell_at_not_selected) */
    void setSellAtNotSelected(bool v) {
        m_sell_at_not_selected = v;
    }

    /** Get whether to force liquidating the unselected sub-systems */
    bool getSellAtNotSelected() const {
        return m_sell_at_not_selected;
    }

    /** Get the turnover rate of every rebalancing day (the turnover amount / the total assets
     * before rebalancing) */
    const std::vector<std::pair<Datetime, double>>& getAdjustTurnover() const {
        return m_adjust_turnover;
    }

    /** Set the driving time axis mode (the axis-mode parameter):
     *  - "kdata" (the default): the date sequence of the input KData of run(kdata) is used as the
     * driving axis (the existing behavior)
     *  - "calendar": the fixed date table (e.g. the market-wide trading calendar) injected by
     * setDateAxis() is used as the driving axis; the input KData degenerates into the query/ktype
     * and the price query context, its own dates no longer drive. An invalid value is warned and it
     * falls back to "kdata". */
    void setAxisMode(const string& mode);

    /** Get the driving time axis mode */
    string getAxisMode() const {
        return tryGetParam<string>("axis-mode", "kdata");
    }

    /** Set the rebalancing mode (aligned with the master PF adjust_mode):
     *  - "query" / "day" (the default): continue the "every N close days" counting judgment of
     * m_adjust_cycle;
     *  - "week" / "month" / "quarter" / "year": expand the rebalancing day table by "the
     * adjust_cycle-th day within the cycle" on the driving axis; An invalid value is warned and it
     * falls back to "query". */
    void setAdjustMode(const string& mode);

    /** Get the rebalancing mode */
    string getAdjustMode() const {
        return tryGetParam<string>("adjust-mode", "query");
    }

    /** Set whether to postpone to the first trading day within the current cycle when the
     * rebalancing day is not a trading day (it takes effect only when week/month/quarter/year are
     * expanded) */
    void setDelayToTradingDay(bool v) {
        setParam<bool>("delay-to-trading-day", v);
    }

    /** Get whether to postpone to the trading day */
    bool getDelayToTradingDay() const {
        return tryGetParam<bool>("delay-to-trading-day", true);
    }

    /** Calculate the rebalancing day set on the given trading day axis by the master Portfolio
     * algorithm (a pure function, convenient for the unit tests and the external preview).
     *  @param dates the sorted trading day axis (usually the driving axis; the
     * suspended/non-trading days should not appear on the axis)
     *  @param mode "week" | "month" | "quarter" | "year" (the other values such as
     * query/day/invalid return empty)
     *  @param adjust_cycle the N-th day within the cycle (<=0 is treated as 1); in the week mode it
     * is dayOfWeek (0=Sunday, 1=Monday ... 6=Saturday)
     *  @param delay_to_trading_day when true it is postponed to the first trading day within the
     * current cycle; when false it only hits when it is exactly the N-th day
     *  @return the ascending deduplicated rebalancing day list
     *  @note Aligned with the master Portfolio::_calculateAdjustDate* behavior
     */
    static DatetimeList calcAdjustDates(const DatetimeList& dates, const string& mode,
                                        int adjust_cycle, bool delay_to_trading_day);

    /** Set the fixed time axis (it is used as the driving axis only when axis-mode == "calendar";
     * when the axis is empty it falls back to the kdata axis with a warning) */
    void setDateAxis(const DatetimeList& dates) {
        m_date_axis = dates;
    }

    /** Get the fixed time axis */
    const DatetimeList& getDateAxis() const {
        return m_date_axis;
    }

    /** Clear the fixed time axis (after clearing, the calendar mode falls back to the kdata axis)
     */
    void clearDateAxis() {
        m_date_axis.clear();
    }

    /** Set the external rebalancing day table (when it is not empty it takes precedence as the
     * rebalancing day criterion, used to map the master adjust_mode =
     * "week"/"month"/"quarter"/"year" and delay_to_trading_day; the input dates are normalized to
     * the zero hour of that day and stored, only the dates hitting the table are regarded as the
     * rebalancing days). */
    void setAdjustDates(const DatetimeList& dates);

    /** Get the external rebalancing day table (already normalized to the zero hour of that day) */
    const std::set<Datetime>& getAdjustDates() const {
        return m_adjust_dates;
    }

    /** Clear the external rebalancing day table (after clearing it falls back to the close-day
     * counting judgment of m_adjust_cycle) */
    void clearAdjustDates() {
        m_adjust_dates.clear();
    }

    /** The mode B quota write-back (written into the sub-system virtual account for the next
     * period; the aggregate sub-system penetrates automatically) */
    virtual void setSubSystemQuota(const SYSPtr& sub_sys, const Datetime& date,
                                   price_t quota) override;

    /** Translate the trades of the direct sub-systems at this moment into the parent suggestions
     * (the glue of the nesting capability) */
    virtual TradeSuggestionList toSuggestions() const override {
        return m_last_suggestions;
    }

public:
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    /** Recursively find the instrument of the system itself or its (nested aggregate) sub-systems,
     * return an empty Stock when not found */
    static Stock _findStock(const SystemPtr& sys);

    // Check whether the candidate subtree (including itself) contains target (used for the circular
    // reference detection)
    static bool _subtreeContains(const SystemPtr& candidate, System* target);

private:
    /** Run by the specified driving axis: when axis is empty the date sequence of the input KData
     * is used as the axis, otherwise axis is the driving axis (when driven by the fixed time axis,
     * the dates on the axis may not exist in the input KData, the suspended/non-trading days do not
     * constitute a gap) */
    void _runAxis(const KData& kdata, const DatetimeList* axis, bool reset, bool resetAll);

    /** Register the parameters of the aggregate system itself (axis-mode / adjust-mode /
     * delay-to-trading-day) */
    void _initAxisParam() {
        setParam<string>("axis-mode", "kdata");
        // Aligned with the master PF adjust_mode / delay_to_trading_day
        setParam<string>("adjust-mode", "query");
        setParam<bool>("delay-to-trading-day", true);
    }

    /** Aggregate a group of trades into a net suggestion by instrument (marked with the source
     * sub-system sys). funds_before is the "before-trade" fund snapshot of the sub-system on that
     * day, used to calculate the three ratios (cash/assets/target_position) of the suggestion. */
    TradeSuggestionList _toSuggestions(const SystemPtr& sys, const TradeRecordList& trades,
                                       const FundsRecord& funds_before,
                                       const Datetime& datetime) const;

    /** Judge whether the given date is a rebalancing day (the rebalancing is executed only on the
     * rebalancing day): the external rebalancing day table takes precedence, then the adjust-mode
     * auto-expanded table, and finally the close-day counting of m_adjust_cycle */
    bool _isAdjustDate(const Datetime& date) const;

    /** Internalize adjust-mode ∈ {week,month,quarter,year} into the rebalancing day table.
     * It takes effect only when the external setAdjustDates() is not injected
     * (m_adjust_dates is empty), the result is written into m_auto_adjust_dates. */
    void _expandAdjustDates(const DatetimeList& axis);

    /** master compatibility: build the rebalancing-day -> cycle-end (the next rebalancing day)
     * mapping on the driving axis, used to drive the cycle-type signals (e.g. SG_Cycle) of the
     * sub-systems on the rebalancing day */
    void _buildCycleEnds(const DatetimeList& axis);

    /** Get the cycle end (the next rebalancing day) of the given rebalancing day; when the day is
     * not in the mapping, fall back to the next day */
    Datetime _getNextCycleEnd(const Datetime& date) const;

    /** Mode B (Fund Allocation, master SimplePortfolio compatibility): reduce the over quota
     * position of the sub-system (the sub book sells on its own and the parent sells the same
     * quantity immediately, the executed records are appended to out_executed). It runs in the
     * reduction phase before the injection phase, so the freed cash is available to the whole
     * portfolio. */
    void _reduceSubSystemToQuota(const SystemPtr& sys, const Datetime& date, price_t quota,
                                 KQuery::KType ktype, TradeRecordList& out_executed);

    /** Mode B (Fund Allocation, master SimplePortfolio compatibility): inject the cash gap when the
     * sub-system is below the quota (limited by the free cash of the parent pool). It runs in the
     * injection phase after the reduction phase. */
    void _injectSubSystemGap(const SystemPtr& sys, const Datetime& date, price_t quota,
                             KQuery::KType ktype);

    /** Mode B (Fund Allocation, master SimplePortfolio compatibility): clear the sub-system shadow
     * account (force selling its holdings and recycling the cash); the parent account holding is
     * cleared by the CLEAR suggestion submitted by the caller */
    void _clearSubSystem(const SystemPtr& sys, const Datetime& date, KQuery::KType ktype);

    /** Mode A (Signal Aggregation): reset the sub-system shadow account on the rebalancing day
     * (force selling its holdings, recycling the cash and injecting the initial signal cash
     * again). The shadow account is a pure signal source: without the reset it runs out of cash
     * after its first position and can never submit a new buy signal, which leaves the parent
     * cash idle and makes the re-entered stocks impossible to buy back. */
    void _resetSubSystemSignalCash(const SystemPtr& sys, const Datetime& date, KQuery::KType ktype);

    /** Execute the converted suggestions on the parent real account (sell first then buy) */
    void _executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                             KQuery::KType ktype, TradeRecordList& out_trades);

    /** The close stage: drive every sub-system to generate signals, merge the "open+close" trades
     * and translate them into the parent suggestions, after the MM allocation the parent orders
     * uniformly; return the actual trades of the parent. Reused by runMoment / runMomentOnClose. */
    TradeRecordList _closePhase(const Datetime& datetime);

    /** The common tail of the close stage: record the rebalancing-day turnover and the trace
     *  output. Shared by the mode A/B tail and the mode C early return. */
    void _finishClosePhase(const Datetime& datetime, const TradeSuggestionList& suggestions,
                           const TradeRecordList& executed, bool is_adjust);

    /** Mode C: the whole close stage of the shared-account mode, i.e. the dual pools of the legacy
     *  Portfolio without AF (running pool + force-sell pool), driven in the pool order and with no
     *  L2 conversion at all. */
    void _closePhaseModeC(const Datetime& datetime, bool is_adjust, const SystemList& se_selected,
                          TradeSuggestionList& suggestions, TradeRecordList& executed);

    /** Mode C: drive one sub-system on the close stage; its trades ARE the trades of the real
     *  account of the parent, so they are surfaced instead of being ordered again. */
    void _runModeCClose(const SystemPtr& sys, const Datetime& datetime,
                        TradeSuggestionList& suggestions, TradeRecordList& executed);

    /** Mode C: add a sub-system to the running pool; true when it is a new admission. */
    bool _addToRunning(const SystemPtr& sys);

    /** Mode C: remove a sub-system from the running pool (both the order and the index). */
    void _removeFromRunning(const SystemPtr& sys);

    /** Delisting: remove the leaf sub-system trading the given stock out of the mode C pools,
     *  searched recursively over the nested aggregates (a nested layer's pools are its own). */
    void _removeStockFromPoolsRecursive(const Stock& stock);

    /** The slot of a sub-system inside m_sys_list (the open trade buffer and the before-trade fund
     *  snapshot are addressed by it); O(1) via m_sub_index. */
    size_t _subIndex(const SystemPtr& sys) const;

    /** master compatibility: when no sub-system was added explicitly, adopt the prototype systems
     *  held by the SE (addStock has already cloned a prototype per instrument and set its stock).
     *  Called by both run() and readyForRun, so that a caller which only configures an SE (the
     *  legacy Portfolio usage, and the live trading entry, which calls readyForRun before any run)
     *  gets its sub-systems materialized. */
    void _adoptSEProtos();

    /** Get the close price of the specified instrument on the specified date (used to price the
     * liquidation suggestion of the unselected sub-system); return 0 when there is no data */
    price_t _getClosePrice(const Datetime& date, const Stock& stock) const;

    /** Get the open price of the specified instrument on the specified date; return 0 when there
     * is no data */
    price_t _getOpenPrice(const Datetime& date, const Stock& stock) const;

    /** Get the KData of the instrument within the run query (with the per-run cache) */
    KData _getStockKData(const Stock& stock) const;

    /** Force selling the parent holdings of the delisted instrument at the open stage (delisting =
     * the last trading day of the instrument is earlier than the current running date) */
    TradeRecordList _forceSellDelisted(const Datetime& date);

private:
    SystemList m_sys_list;
    string m_path;                // The hierarchy path, e.g. I/D/A
    size_t m_close_day_index{0};  // The close-day counter, used for the rebalancing cycle judgment
    price_t m_sub_init_cash{100000.0};  // The signal cash of the sub-system shadow account (mode A,
                                        // reset on every rebalancing day; unused in mode B)
    int m_adjust_cycle{
      1};  // The rebalancing cycle (days); <=1 means rebalancing on every close day
    bool m_trade_on_close{true};  // Whether to execute the rebalancing orders at the close stage
    AllocateFundsPtr m_af{AF_EqualWeight()};  // The portfolio-level fund allocation (AF, including
                                              // L1/L2/L3); the running mode is held by it
    std::shared_ptr<SelectorBase> m_se;       // The trading object selector (optional)
    bool m_sell_at_not_selected{true};  // Whether to force liquidating the unselected sub-systems
                                        // (it takes effect only after SE is set)
    std::vector<std::pair<Datetime, double>>
      m_adjust_turnover;  // The rebalancing-day turnover rate (the turnover amount / the total
                          // assets before rebalancing)
    TradeSuggestionList m_last_suggestions;  // The parent suggestion produced by the last close
    std::vector<TradeRecordList>
      m_open_trades;  // The open trades of every sub-system on that day (the delayed requests
                      // fulfilled), used for the close aggregation
    std::vector<FundsRecord>
      m_sub_funds_before;  // The "before-trade" fund snapshot of every sub-system on that day, used
                           // by _toSuggestions to calculate the three ratios (runtime state, not
                           // serialized)
    Datetime m_open_trades_date;   // The trading day to which m_open_trades/m_sub_funds_before
                                   // belong; the close stage uses it to prevent out-of-bounds and
                                   // cross-day residue (runtime state, not serialized)
    Datetime m_signal_reset_date;  // The last day when the mode A (Signal Aggregation) signal
                                   // cash reset ran; it
                                   // prevents multiple resets within the same trading day (the
                                   // close stage may be driven several times a day) (runtime
                                   // state, not serialized)
    TradeSuggestionList m_pending_suggestions;  // The suggestions accumulated on the
                                                // non-rebalancing days (runtime, not serialized)
    TradeSuggestionList m_open_pending_suggestions;  // The converted suggestions to be executed at
                                                     // the next open when trade_on_close=false
    std::set<System*> m_shadow_sys;  // The sub-systems whose shadow account has been created
                                     // (runtime, not serialized)
    // Mode C: the running pool. m_running_order keeps the legacy insertion order, which decides
    // who gets the shared cash first (the cash competition order of the legacy Portfolio);
    // m_running_set is the membership index of the very same content.
    std::list<SystemPtr> m_running_order;
    std::set<System*> m_running_set;
    // Mode C: the force-sell pool (the legacy m_force_sell_sys_list). Runtime state, neither
    // serialized nor copied by clone.
    SystemList m_force_sell_list;
    // The slot of every sub-system inside m_sys_list (the O(1) backing of _subIndex). Runtime
    // state: maintained by add(), rebuilt by readyForRun, neither serialized nor copied by clone.
    std::unordered_map<System*, size_t> m_sub_index;
    mutable std::unordered_map<string, KData>
      m_kdata_cache;  // The instrument KData cache within the run query (runtime, not serialized)
    DatetimeList m_date_axis;           // The fixed time axis: the driving date table when
                                        // axis-mode="calendar" (runtime state, not serialized)
    std::set<Datetime> m_adjust_dates;  // The external rebalancing day table (normalized to the
                                        // zero hour of that day; when not empty it takes precedence
                                        // over m_adjust_cycle, runtime state, not serialized)
    std::set<Datetime>
      m_auto_adjust_dates;  // The rebalancing day table auto-expanded by adjust-mode (runtime
                            // state, not serialized, does not override the external injection)
    std::map<Datetime, Datetime>
      m_cycle_ends;  // The rebalancing-day -> cycle-end (the next rebalancing day) mapping, used by
                     // the cycle-type signal driving (runtime state, rebuilt with each run)

//========================================
// Serialization support
//========================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(System);
        ar& BOOST_SERIALIZATION_NVP(m_sys_list);
        ar& BOOST_SERIALIZATION_NVP(m_path);
        ar& BOOST_SERIALIZATION_NVP(m_sub_init_cash);
        ar& BOOST_SERIALIZATION_NVP(m_adjust_cycle);
        ar& BOOST_SERIALIZATION_NVP(m_trade_on_close);
        ar& BOOST_SERIALIZATION_NVP(m_sell_at_not_selected);
        if (version < 1) {
            // Old archives stored the running mode here (it has been migrated to
            // AllocateFundsBase); write it back into the AF after being read.
            string legacy_mode = "A";
            ar& boost::serialization::make_nvp("m_mode", legacy_mode);
            if (m_af) {
                m_af->setMode(legacy_mode);
            }
        } else {
            // The fund allocation instance (including L1/L2/L3 and the running mode) is
            // serialized together with the aggregate system
            ar& BOOST_SERIALIZATION_NVP(m_af);
        }
        ar& BOOST_SERIALIZATION_NVP(m_se);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<MultiSystem> MultiSystemPtr;

/**
 * master compatibility alias: in master PF_Simple / PF_WithoutAF return PortfolioPtr.
 * Here PF is a preset configuration of MultiSystem, this alias is kept so that the existing
 * `PortfolioPtr pf = PF_Simple(...)` keeps compiling (the Portfolio class methods are no longer
 * supported).
 * @ingroup Portfolio
 */
using PortfolioPtr = MultiSystemPtr;
using PFPtr = MultiSystemPtr;

}  // namespace hku

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_VERSION(::hku::MultiSystem, 1)
#endif
