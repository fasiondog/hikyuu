/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  Recursive combination refactoring: the aggregate trading system (portfolio backtesting)
 *  It holds multiple sub-systems (single-security or nested aggregate), drives and aggregates the orders at the open/close stages respectively.
 *  Stage 3: dual modes (A/B) + arbitrary nesting + MM L1/L2/L3 + rebalancing cycle + hierarchy path.
 *  Mode A (the default): the parent gives the sub-systems a "shadow account", the parent allocates and orders by weight uniformly (functionally equivalent to the PF signal aggregation).
 */

#pragma once
#include <set>
#include "../System.h"
#include "../../../trade_manage/crt/crtTM.h"
#include "../../moneymanager/crt/MM_Nothing.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

class SelectorBase;  // Forward declaration, to avoid a circular include with SelectorBase.h (which includes System.h)

class HKU_API MultiSystem : public System {
public:
    MultiSystem() : System() {
        _initAxisParam();
        // The default MM of the aggregate parent system is mode A (signal aggregation, the base allocate implements the equal weight allocation)
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

    /** Add a sub-system (with the circular reference detection, it rejects the node that contains itself or an existing node) */
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

    /** master compatibility overload: equivalent to master `Portfolio::run(query)`, it runs with the market trading calendar as the driving axis.
     *  - Driving axis: the explicitly injected fixed time axis (axis-mode == "calendar" and not empty) takes precedence,
     *            otherwise the StockManager market trading calendar `get_trading_calendar(query)` is used (the SH market by default);
     *  - Context KData: it only provides the query/ktype and the price query context, its own instrument `getStock()` takes precedence,
     *            then the first (recursive) sub-system instrument, and finally it degenerates to the KData of the calendar benchmark index (e.g. sh000001);
     *  - The axis-mode / fixed time axis parameters are not modified, the calendar axis only drives this run.
     *  @param query the query condition (also used as the query context of the sub-systems and the prices)
     *  @param reset whether to reset before running (forwarded to every sub-system)
     *  @param resetAll whether to force a full reset before running
     *  @note when ktype is not the daily line, consistent with master, adjust-mode is required to be query/day (the calendar axis is a daily line sequence). */
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

    /** Set the fund allocation instance (the portfolio-level fund allocation AF, carrying the three algorithm parts L1/L2/L3) */
    void setAF(const AllocateFundsPtr& af) {
        if (af) {
            m_af = af;
        }
    }

    /** Get the fund allocation instance */
    const AllocateFundsPtr& getAF() const {
        return m_af;
    }

    /** Set the running mode: A (signal aggregation, the default) / B (fund allocation / FOF-MOM).
     *  In mode B the parent produces the real quota through L1 and writes it back to every sub-system on the rebalancing day.
     *  @note The mode is held by AF (the only source), this method writes it directly into the current AF. */
    void setMode(const string& mode) {
        if (m_af) {
            m_af->setMode(mode);
        }
    }

    /** Get the running mode (from AF) */
    const string& getMode() const;

    /** Set the initial fund of the sub-system shadow account (a fixed value in mode A / the initial quota in mode B) */
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

    /** Set the trading object selector (optional; after it is set only the sub-systems selected by SE run, the unselected ones can be liquidated) */
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

    /** Get the turnover rate of every rebalancing day (the turnover amount / the total assets before rebalancing) */
    const std::vector<std::pair<Datetime, double>>& getAdjustTurnover() const {
        return m_adjust_turnover;
    }

    /** Set the driving time axis mode (the axis-mode parameter):
     *  - "kdata" (the default): the date sequence of the input KData of run(kdata) is used as the driving axis (the existing behavior)
     *  - "calendar": the fixed date table (e.g. the market-wide trading calendar) injected by setDateAxis() is used as the driving axis;
     *               the input KData degenerates into the query/ktype and the price query context, its own dates no longer drive.
     *  An invalid value is warned and it falls back to "kdata". */
    void setAxisMode(const string& mode);

    /** Get the driving time axis mode */
    string getAxisMode() const {
        return tryGetParam<string>("axis-mode", "kdata");
    }

    /** Set the rebalancing mode (taking over the master PF adjust_mode, see design.md §4.3):
     *  - "query" / "day" (the default): continue the "every N close days" counting judgment of m_adjust_cycle;
     *  - "week" / "month" / "quarter" / "year": expand the rebalancing day table by "the adjust_cycle-th day within the cycle" on the driving axis;
     *  An invalid value is warned and it falls back to "query". */
    void setAdjustMode(const string& mode);

    /** Get the rebalancing mode */
    string getAdjustMode() const {
        return tryGetParam<string>("adjust-mode", "query");
    }

    /** Set whether to postpone to the first trading day within the current cycle when the rebalancing day is not a trading day (it takes effect only when week/month/quarter/year are expanded) */
    void setDelayToTradingDay(bool v) {
        setParam<bool>("delay-to-trading-day", v);
    }

    /** Get whether to postpone to the trading day */
    bool getDelayToTradingDay() const {
        return tryGetParam<bool>("delay-to-trading-day", true);
    }

    /** Calculate the rebalancing day set on the given trading day axis by the master Portfolio algorithm (a pure function, convenient for the unit tests and the external preview).
     *  @param dates the sorted trading day axis (usually the driving axis; the suspended/non-trading days should not appear on the axis)
     *  @param mode "week" | "month" | "quarter" | "year" (the other values such as query/day/invalid return empty)
     *  @param adjust_cycle the N-th day within the cycle (<=0 is treated as 1); in the week mode it is dayOfWeek (0=Sunday, 1=Monday ... 6=Saturday)
     *  @param delay_to_trading_day when true it is postponed to the first trading day within the current cycle; when false it only hits when it is exactly the N-th day
     *  @return the ascending deduplicated rebalancing day list
     *  @note Aligned with the master Portfolio::_calculateAdjustDate* behavior (see design.md §4.3) */
    static DatetimeList calcAdjustDates(const DatetimeList& dates, const string& mode,
                                        int adjust_cycle, bool delay_to_trading_day);

    /** Set the fixed time axis (it is used as the driving axis only when axis-mode == "calendar"; when the axis is empty it falls back to the kdata axis with a warning) */
    void setDateAxis(const DatetimeList& dates) {
        m_date_axis = dates;
    }

    /** Get the fixed time axis */
    const DatetimeList& getDateAxis() const {
        return m_date_axis;
    }

    /** Clear the fixed time axis (after clearing, the calendar mode falls back to the kdata axis) */
    void clearDateAxis() {
        m_date_axis.clear();
    }

    /** Set the external rebalancing day table (when it is not empty it takes precedence as the rebalancing day criterion, used to map the master
     *  adjust_mode = "week"/"month"/"quarter"/"year" and delay_to_trading_day;
     *  the input dates are normalized to the zero hour of that day and stored, only the dates hitting the table are regarded as the rebalancing days). */
    void setAdjustDates(const DatetimeList& dates);

    /** Get the external rebalancing day table (already normalized to the zero hour of that day) */
    const std::set<Datetime>& getAdjustDates() const {
        return m_adjust_dates;
    }

    /** Clear the external rebalancing day table (after clearing it falls back to the close-day counting judgment of m_adjust_cycle) */
    void clearAdjustDates() {
        m_adjust_dates.clear();
    }

    /** The mode B quota write-back (written into the sub-system virtual account for the next period; the aggregate sub-system penetrates automatically) */
    virtual void setSubSystemQuota(const SYSPtr& sub_sys, const Datetime& date,
                                   price_t quota) override;

    /** Translate the trades of the direct sub-systems at this moment into the parent suggestions (the glue of the nesting capability) */
    virtual TradeSuggestionList toSuggestions() const override {
        return m_last_suggestions;
    }

public:
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    SystemList m_sys_list;
    string m_path;                 // The hierarchy path, e.g. I/D/A
    size_t m_close_day_index{0};    // The close-day counter, used for the rebalancing cycle judgment
    price_t m_sub_init_cash{100000.0};  // The initial fund of the sub-system shadow account (mode A)
    int m_adjust_cycle{1};          // The rebalancing cycle (days); <=1 means rebalancing on every close day
    bool m_trade_on_close{true};    // Whether to execute the rebalancing orders at the close stage
    AllocateFundsPtr m_af{AF_EqualWeight()};  // The portfolio-level fund allocation (AF, including L1/L2/L3); the running mode is held by it
    std::shared_ptr<SelectorBase> m_se;  // The trading object selector (optional)
    bool m_sell_at_not_selected{true};   // Whether to force liquidating the unselected sub-systems (it takes effect only after SE is set)
    std::vector<std::pair<Datetime, double>> m_adjust_turnover;  // The rebalancing-day turnover rate (the turnover amount / the total assets before rebalancing)
    TradeSuggestionList m_last_suggestions;  // The parent suggestion produced by the last close
    std::vector<TradeRecordList> m_open_trades;  // The open trades of every sub-system on that day (the delayed requests fulfilled), used for the close aggregation
    std::vector<FundsRecord> m_sub_funds_before;  // The "before-trade" fund snapshot of every sub-system on that day, used by _toSuggestions to calculate the three ratios (runtime state, not serialized)
    Datetime m_open_trades_date;  // The trading day to which m_open_trades/m_sub_funds_before belong; the close stage uses it to prevent out-of-bounds and cross-day residue (runtime state, not serialized)
    DatetimeList m_date_axis;     // The fixed time axis: the driving date table when axis-mode="calendar" (runtime state, not serialized)
    std::set<Datetime> m_adjust_dates;  // The external rebalancing day table (normalized to the zero hour of that day; when not empty it takes precedence over m_adjust_cycle, runtime state, not serialized)
    std::set<Datetime> m_auto_adjust_dates;  // The rebalancing day table auto-expanded by adjust-mode (runtime state, not serialized, does not override the external injection)

    /** Run by the specified driving axis: when axis is empty the date sequence of the input KData is used as the axis, otherwise axis is the driving axis
     *  (when driven by the fixed time axis, the dates on the axis may not exist in the input KData, the suspended/non-trading days do not constitute a gap) */
    void _runAxis(const KData& kdata, const DatetimeList* axis, bool reset, bool resetAll);

    /** Recursively find the instrument of the system itself or its (nested aggregate) sub-systems, return an empty Stock when not found */
    static Stock _findStock(const SystemPtr& sys);

    /** Register the parameters of the aggregate system itself (axis-mode / adjust-mode / delay-to-trading-day) */
    void _initAxisParam() {
        setParam<string>("axis-mode", "kdata");
        // v5: take over the master PF adjust_mode / delay_to_trading_day (see docs/design/pf_af_compat/design.md §4.3)
        setParam<string>("adjust-mode", "query");
        setParam<bool>("delay-to-trading-day", true);
    }

    // Check whether the candidate subtree (including itself) contains target (used for the circular reference detection)
    static bool _subtreeContains(const SystemPtr& candidate, System* target);

    /** Aggregate a group of trades into a net suggestion by instrument (marked with the source sub-system sys).
     *  funds_before is the "before-trade" fund snapshot of the sub-system on that day, used to calculate the three ratios (cash/assets/target_position) of the suggestion. */
    TradeSuggestionList _toSuggestions(const SystemPtr& sys, const TradeRecordList& trades,
                                       const FundsRecord& funds_before) const;

    /** Judge whether the given date is a rebalancing day (the rebalancing is executed only on the rebalancing day):
     *  the external rebalancing day table takes precedence, then the adjust-mode auto-expanded table, and finally the close-day counting of m_adjust_cycle */
    bool _isAdjustDate(const Datetime& date) const;

    /** v5: internalize adjust-mode ∈ {week,month,quarter,year} into the rebalancing day table (design.md §4.3).
     *  It takes effect only when the external setAdjustDates() is not injected (m_adjust_dates is empty), the result is written into m_auto_adjust_dates. */
    void _expandAdjustDates(const DatetimeList& axis);

    /** Execute the converted suggestions on the parent real account (sell first then buy) */
    void _executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                             KQuery::KType ktype, TradeRecordList& out_trades);

    /** The close stage: drive every sub-system to generate signals, merge the "open+close" trades and translate them into the parent suggestions,
     *  after the MM allocation the parent orders uniformly; return the actual trades of the parent. Reused by runMoment / runMomentOnClose. */
    TradeRecordList _closePhase(const Datetime& datetime);

    /** Get the close price of the specified instrument on the specified date (used to price the liquidation suggestion of the unselected sub-system); return 0 when there is no data */
    price_t _getClosePrice(const Datetime& date, const Stock& stock) const;

    /** Force selling the parent holdings of the delisted instrument at the open stage (delisting = the last trading day of the instrument is earlier than the current running date) */
    TradeRecordList _forceSellDelisted(const Datetime& date);

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
            // v5 compatibility: this position was m_mode in the old archives (the running mode has been migrated to AllocateFundsBase), it is written back into AF after being read.
            string legacy_mode = "A";
            ar& boost::serialization::make_nvp("m_mode", legacy_mode);
            if (m_af) {
                m_af->setMode(legacy_mode);
            }
        } else {
            // v5: the fund allocation instance (including L1/L2/L3 and the running mode) is serialized together with the aggregate system
            ar& BOOST_SERIALIZATION_NVP(m_af);
        }
        ar& BOOST_SERIALIZATION_NVP(m_se);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<MultiSystem> MultiSystemPtr;

/**
 * master compatibility alias: in master PF_Simple / PF_WithoutAF return PortfolioPtr.
 * In feature/next PF is a concrete implementation of MultiSystem (see docs/design/pf_af_compat/design.md §4.5),
 * this alias is kept so that the existing `PortfolioPtr pf = PF_Simple(...)` keeps compiling (the Portfolio class methods are no longer supported).
 * @ingroup Portfolio
 */
using PortfolioPtr = MultiSystemPtr;

}  // namespace hku

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_VERSION(::hku::MultiSystem, 1)
#endif
