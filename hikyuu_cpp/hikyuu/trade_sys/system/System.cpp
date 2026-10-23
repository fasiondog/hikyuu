/*
 * SystemBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include <algorithm>
#include <cmath>
#include "hikyuu/global/sysinfo.h"
#include "System.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::System)
#endif

namespace hku {

// Map a price in the adjusted KData coordinate of the current bar back to the original
// (unadjusted) coordinate through the affine transformation determined by the bar range;
// on a one-price board fall back to the original low price
static price_t mapAdjustedToOriginal(const KRecord& today, const KRecord& src_today,
                                     price_t price) {
    if (today.highPrice == today.lowPrice) {
        return src_today.lowPrice;
    }
    if (src_today.highPrice == src_today.lowPrice) {
        return src_today.lowPrice;
    }
    price_t adjust = (price - today.lowPrice) / (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                     src_today.lowPrice;
    HKU_WARN_IF(!std::isfinite(adjust),
                "mapAdjustedToOriginal: non-finite adjusted price, treated as no stop-loss");
    return adjust >= 0.0 ? adjust : 0.0;
}

HKU_API std::ostream& operator<<(std::ostream& os, const System& sys) {
    os << sys.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SystemPtr& sys) {
    if (sys) {
        os << sys->str();
    } else {
        os << "System(NULL)";
    }
    return os;
}

string System::str() const {
    std::ostringstream os;
    string strip(",\n");
    string space("  ");
    os << "System{\n"
       << space << name() << strip << space << getTO().getQuery() << strip << space << getStock()
       << strip << space << getParameter() << strip << space << getEV() << strip << space << getCN()
       << strip << space << getMM() << strip << space << getSG() << strip << space << getST()
       << strip << space << getTP() << strip << space << getPG() << strip << space << getSP()
       << strip << space << (getTM() ? getTM()->str() : "TradeManager(NULL)") << strip << "}";
    return os.str();
}

System::System()
: m_name("SYS_Simple"),
  m_calculated(false),
  m_pre_ev_valid(true),  // must true
  m_pre_cn_valid(true),  // must true
  m_lastTakeProfit(0.0),
  m_lastShortTakeProfit(0.0) {
    initParam();
}

System::System(const string& name)
: m_name(name),
  m_calculated(false),
  m_pre_ev_valid(true),
  m_pre_cn_valid(true),
  m_lastTakeProfit(0.0),
  m_lastShortTakeProfit(0.0) {
    initParam();
}

System::System(const TradeManagerPtr& tm, const MoneyManagerPtr& mm, const EnvironmentPtr& ev,
               const ConditionPtr& cn, const SignalPtr& sg, const StoplossPtr& st,
               const StoplossPtr& tp, const ProfitGoalPtr& pg, const SlippagePtr& sp,
               const string& name)
: m_tm(tm),
  m_mm(mm),
  m_ev(ev),
  m_cn(cn),
  m_sg(sg),
  m_st(st),
  m_tp(tp),
  m_pg(pg),
  m_sp(sp),
  m_name(name),
  m_calculated(false),
  m_pre_ev_valid(true),
  m_pre_cn_valid(true),
  m_lastTakeProfit(0.0),
  m_lastShortTakeProfit(0.0) {
    initParam();
}

System::~System() {}

void System::initParam() {
    setParam<bool>("trace", false);

    // The limit of the consecutive delayed trade requests; it must be >= 0 and 0 means only one
    // delay is allowed
    setParam<int>("max_delay_count", 3);

    // Whether to trade at the open of the next bar with a delay
    setParam<bool>("buy_delay",
                   true);  // Without a delay the close price of the current bar is used
    setParam<bool>("sell_delay", true);

    // In the delayed case, whether to calculate the new stop-loss / take-profit / goal price with
    // the current price or to use the result calculated last time
    setParam<bool>("delay_use_current_price", true);
    setParam<bool>("tp_monotonic", true);  // The take-profit increases monotonically
    setParam<int>("tp_delay_n", 1);        // Number of the days of the take-profit delay judgment
    setParam<bool>("ignore_sell_sg", false);  // Ignore the sell signal and use the other ways only

    // Whether a trade can be done when the high price equals the low price
    setParam<bool>("can_trade_when_high_eq_low", false);

    // Whether to use the market environment (EV) for the initial position building
    setParam<bool>("ev_open_position", false);

    // Whether to use the system valid condition for the initial position building
    setParam<bool>("cn_open_position", false);

    // Whether to support borrowing cash (financing) when the cash is not enough
    setParam<bool>("support_borrow_cash", false);

    // Whether to support borrowing securities (securities lending) when there is no position
    setParam<bool>("support_borrow_stock", false);

    // The following parameters control the sharing strategy of the parts and affect the clone and
    // reset operations: a shared part is neither cloned nor reset
    setParam<bool>("shared_tm", false);
    setParam<bool>("shared_ev", true);
    setParam<bool>("shared_cn", false);
    setParam<bool>("shared_sg", false);
    setParam<bool>("shared_mm", false);
    setParam<bool>("shared_st", false);
    setParam<bool>("shared_tp", false);
    setParam<bool>("shared_pg", false);
    setParam<bool>("shared_sp", false);
}

void System::setNotSharedAll() {
    setParam<bool>("shared_tm", false);
    setParam<bool>("shared_ev", false);
    setParam<bool>("shared_cn", false);
    setParam<bool>("shared_sg", false);
    setParam<bool>("shared_mm", false);
    setParam<bool>("shared_st", false);
    setParam<bool>("shared_tp", false);
    setParam<bool>("shared_pg", false);
    setParam<bool>("shared_sp", false);
}

void System::baseCheckParam(const string& name) const {
    if ("max_delay_count" == name) {
        HKU_ASSERT(getParam<int>("max_delay_count") >= 0);
    } else if ("tp_delay_n" == name) {
        HKU_ASSERT(getParam<int>("tp_delay_n") >= 0);
    } else if ("trace" == name) {
        if (getParam<bool>("trace") && pythonInJupyter()) {
            HKU_THROW("{}", htr("You can't trace in jupyter!"));
        }
    }
}

void System::paramChanged() {
    m_calculated = false;
}

void System::reset() {
    if (m_tm && !getParam<bool>("shared_tm"))
        m_tm->reset();
    if (m_ev && !getParam<bool>("shared_ev"))
        m_ev->reset();
    if (m_cn && !getParam<bool>("shared_cn"))
        m_cn->reset();
    if (m_mm && !getParam<bool>("shared_mm"))
        m_mm->reset();
    if (m_sg && !getParam<bool>("shared_sg"))
        m_sg->reset();
    if (m_st && !getParam<bool>("shared_st"))
        m_st->reset();
    if (m_tp && !getParam<bool>("shared_tp"))
        m_tp->reset();
    if (m_pg && !getParam<bool>("shared_pg"))
        m_pg->reset();
    if (m_sp && !getParam<bool>("shared_sp"))
        m_sp->reset();

    // m_stock / m_kdata / m_src_kdata must not be reset; they are needed by the subsequent
    // Portfolio; in a sense the sys instance and the stock are bound one to one, and once a sys
    // instance is bound to a stock it should not be reset unless it is changed actively
    //  m_stock

    m_calculated = false;
    m_pre_ev_valid = m_ev ? false : true;
    m_pre_cn_valid = m_cn ? false : true;

    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequestList.clear();
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
    m_buyShortRequestList.clear();

    _reset();
}

void System::forceResetAll() {
    if (m_tm && !getParam<bool>("shared_tm"))
        m_tm->reset();
    if (m_ev)
        m_ev->reset();
    if (m_cn)
        m_cn->reset();
    if (m_mm)
        m_mm->reset();
    if (m_sg)
        m_sg->reset();
    if (m_st)
        m_st->reset();
    if (m_tp)
        m_tp->reset();
    if (m_pg)
        m_pg->reset();
    if (m_sp)
        m_sp->reset();

    // Clean up the trading object
    m_stock = Null<Stock>();
    m_src_kdata = Null<KData>();
    m_kdata = Null<KData>();

    m_calculated = false;
    m_pre_ev_valid = m_ev ? false : true;
    m_pre_cn_valid = m_cn ? false : true;

    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequestList.clear();
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
    m_buyShortRequestList.clear();

    _forceResetAll();
}

void System::setTO(const KData& kdata) {
    if (m_kdata != kdata) {
        m_calculated = false;
        m_kdata = kdata;
    }

    HKU_TRACE_IF_RETURN(m_calculated, void(), "No need to calcule!");

    m_stock = m_kdata.getStock();
    KQuery query = m_kdata.getQuery();
    if (m_stock.isNull() || query.recoverType() == KQuery::NO_RECOVER) {
        m_src_kdata = m_kdata;
    } else {
        KQuery no_recover_query = query;
        no_recover_query.recoverType(KQuery::NO_RECOVER);
        m_src_kdata = m_stock.getKData(no_recover_query);
    }
    HKU_ASSERT(m_kdata.size() == m_src_kdata.size());

    HKU_WARN_IF(
      query.recoverType() == KQuery::FORWARD || query.recoverType() == KQuery::EQUAL_FORWARD,
      htr("You are using forward or equal_forward adjusted K-line data, which introduces "
          "look-ahead bias!"));

    // sg->setTO must come before cn->setTO, because cn uses sg; this prevents sg from being
    // calculated twice
    if (m_sg)
        m_sg->setTO(m_kdata);  // The adjusted KData is passed in
    if (m_cn)
        m_cn->setTO(m_kdata);  // The adjusted KData is passed in
    if (m_st)
        m_st->setTO(m_kdata);  // The adjusted KData is passed in
    if (m_tp)
        m_tp->setTO(m_kdata);  // The adjusted KData is passed in
    if (m_pg)
        m_pg->setTO(m_src_kdata);  // The original KData without adjustment is passed in
    if (m_sp)
        m_sp->setTO(m_src_kdata);  // The original KData without adjustment is passed in

    if (m_ev)
        m_ev->setQuery(query);
    if (m_mm)
        m_mm->setQuery(query);
}

bool System::hasPythonPart() const {
    if (m_is_python_object || (m_tm && m_tm->hasPythonPart()) || (m_ev && m_ev->hasPythonPart()) ||
        (m_cn && m_cn->hasPythonPart()) || (m_sg && m_sg->hasPythonPart()) ||
        (m_st && m_st->hasPythonPart()) || (m_tp && m_tp->hasPythonPart()) ||
        (m_mm && m_mm->hasPythonPart()) || (m_pg && m_pg->hasPythonPart()) ||
        (m_sp && m_sp->hasPythonPart())) {
        return true;
    }
    for (const auto& sub : getSubSystemList()) {
        if (sub && sub->hasPythonPart()) {
            return true;
        }
    }
    return false;
}

SystemPtr System::clone() {
    SystemPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    if (m_tm)
        p->m_tm = getParam<bool>("shared_tm") ? m_tm : m_tm->clone();
    if (m_ev)
        p->m_ev = getParam<bool>("shared_ev") ? m_ev : m_ev->clone();
    if (m_mm)
        p->m_mm = getParam<bool>("shared_mm") ? m_mm : m_mm->clone();
    if (m_cn)
        p->m_cn = getParam<bool>("shared_cn") ? m_cn : m_cn->clone();
    if (m_sg)
        p->m_sg = getParam<bool>("shared_sg") ? m_sg : m_sg->clone();
    if (m_st)
        p->m_st = getParam<bool>("shared_st") ? m_st : m_st->clone();
    if (m_tp)
        p->m_tp = getParam<bool>("shared_tp") ? m_tp : m_tp->clone();
    if (m_pg)
        p->m_pg = getParam<bool>("shared_pg") ? m_pg : m_pg->clone();
    if (m_sp)
        p->m_sp = getParam<bool>("shared_sp") ? m_sp : m_sp->clone();

    p->m_is_python_object = m_is_python_object;
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_stock = m_stock;
    p->m_kdata = m_kdata;
    p->m_src_kdata = m_src_kdata;
    p->m_path = m_path;

    p->m_calculated = m_calculated;
    p->m_pre_ev_valid = m_pre_ev_valid;
    p->m_pre_cn_valid = m_pre_cn_valid;

    p->m_trade_list = m_trade_list;
    p->m_lastTakeProfit = m_lastTakeProfit;
    p->m_lastShortTakeProfit = m_lastShortTakeProfit;

    p->m_buyRequestList = m_buyRequestList;
    p->m_sellRequestList = m_sellRequestList;
    p->m_sellShortRequestList = m_sellShortRequestList;
    p->m_buyShortRequestList = m_buyShortRequestList;

    return p;
}

void System::_buyNotifyAll(const TradeRecord& record) {
    if (m_mm)
        m_mm->buyNotify(record);
    if (m_pg)
        m_pg->buyNotify(record);
}

void System::_sellNotifyAll(const TradeRecord& record) {
    if (m_mm)
        m_mm->sellNotify(record);
    if (m_pg)
        m_pg->sellNotify(record);
}

void System::readyForRun() {
    HKU_CHECK(m_tm, "Not setTradeManager! {}", name());
    HKU_CHECK(m_mm, "Not setMoneyManager! {}", name());
    HKU_CHECK(m_sg, "Not setSignal! {}", name());

    // When a market environment (EV) strategy exists, the default previous-day market valid
    // flag must be set to false, because whether the market is valid must be judged entirely by the
    // market environment strategy
    if (m_ev)
        m_pre_ev_valid = false;

    if (m_cn) {
        m_cn->setTM(m_tm);
        m_cn->setSG(m_sg);
        m_pre_cn_valid = false;  // The default previous-day market valid flag is set to false
    }

    m_mm->setTM(m_tm);
    if (m_pg)
        m_pg->setTM(m_tm);
    if (m_st)
        m_st->setTM(m_tm);
    if (m_tp)
        m_tp->setTM(m_tm);

    m_tm->setParam<bool>("support_borrow_cash", getParam<bool>("support_borrow_cash"));
    m_tm->setParam<bool>("support_borrow_stock", getParam<bool>("support_borrow_stock"));
}

void System::run(const KQuery& query, bool reset, bool resetAll) {
    HKU_CHECK(!m_stock.isNull(), "m_stock is NULL!");
    KData kdata = m_stock.getKData(query);
    run(kdata, reset, resetAll);
}

void System::run(const Stock& stock, const KQuery& query, bool reset, bool resetAll) {
    HKU_CHECK(!stock.isNull(), "stock is NULL!");
    KData kdata = stock.getKData(query);
    run(kdata, reset, resetAll);
}

void System::run(const KData& kdata, bool reset, bool resetAll) {
    // reset must come before readyForRun, otherwise m_pre_cn_valid and m_pre_ev_valid would be
    // assigned wrong initial values
    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    HKU_DEBUG_IF_RETURN(m_calculated && m_kdata == kdata, void(), "Not need calculate.");

    readyForRun();

    bool trace = getParam<bool>("trace");
    setTO(kdata);
    size_t total = m_kdata.size();
    auto const* ks = m_kdata.data();
    auto const* src_ks = m_src_kdata.data();
    HKU_ASSERT(m_kdata.size() == m_src_kdata.size());

    // Adapt to the deviation that may be caused by synchronizing the asset information when running
    // in the strategy mode
    Datetime tm_init_datetime = m_tm->initDatetime();
    Datetime tm_last_datetime = m_tm->lastDatetime();
    if (KQuery::getKTypeInSeconds(m_kdata.getQuery().kType()) >= 86400) {
        tm_init_datetime = tm_init_datetime.startOfDay();
        tm_last_datetime = tm_last_datetime.startOfDay();
    }

    for (size_t i = 0; i < total; ++i) {
        if (ks[i].datetime >= tm_init_datetime && ks[i].datetime >= tm_last_datetime) {
            auto tr = _runMoment(ks[i], src_ks[i]);
            if (trace) {
                HKU_INFO_IF(!tr.isNull(), "{}", tr);
                PositionRecord position = m_tm->getPosition(ks[i].datetime, m_stock);
                FundsRecord funds = m_tm->getFunds(ks[i].datetime, m_kdata.getQuery().kType());
                if (position.number > 0.0) {
                    // clang-format off
                    HKU_INFO("+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+");
                    HKU_INFO("| total       | cash        | profit      | market      | position    | close price | stoploss    | goal price  | total cost  |");
                    HKU_INFO("+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+");
                    HKU_INFO("| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}| {:<12.2f}|",
                        funds.total_assets(), funds.cash, funds.profit(), funds.market_value, position.number, src_ks[i].closePrice,
                        position.stoploss, position.goalPrice, position.totalCost);
                    HKU_INFO("+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+");
                    // clang-format on
                }
            }
        }
    }
    m_calculated = true;
}

void System::clearDelayBuyRequest() {
    m_buyRequestList.clear();
    m_buyShortRequestList.clear();
}

void System::clearDelaySellRequest() {
    m_sellRequestList.clear();
    m_sellShortRequestList.clear();
}

const std::vector<std::shared_ptr<System>>& System::getSubSystemList() const {
    static SystemList empty;
    return empty;
}

MomentResult System::runMoment(const Datetime& datetime) {
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), result);

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    KQuery::KType ktype = m_kdata.getQuery().kType();

    result.datetime = datetime;
    result.funds_before_open = m_tm->getFunds(datetime, ktype);

    TradeRecord tr_open = _runMomentOnOpen(today, src_today);
    if (!tr_open.isNull()) {
        result.tradesOnOpen.push_back(tr_open);
    }

    result.funds_before_close = m_tm->getFunds(datetime, ktype);

    TradeRecord tr_close = _runMomentOnClose(today, src_today);
    if (!tr_close.isNull()) {
        result.tradesOnClose.push_back(tr_close);
    }

    result.funds = m_tm->getFunds(datetime, ktype);

    PositionRecord position = m_tm->getPosition(datetime, m_stock);
    if (position.number > 0.0) {
        result.positions.push_back(position);
    }
    PositionRecord short_position = m_tm->getShortPosition(m_stock);
    if (short_position.number > 0.0) {
        result.positions.push_back(short_position);
    }
    // Collect the pending delayed requests, for the upper layer to perceive the requests
    // fulfilled at the next open
    _collectDelayRequests(result);
    return result;
}

MomentResult System::runMomentOnOpen(const Datetime& datetime) {
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), result);

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    TradeRecord tr = _runMomentOnOpen(today, src_today);
    if (!tr.isNull()) {
        result.tradesOnOpen.push_back(tr);
    }
    return result;
}

MomentResult System::runMomentOnClose(const Datetime& datetime) {
    MomentResult result;
    HKU_IF_RETURN(!m_tm, result);
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), result);

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    TradeRecord tr = _runMomentOnClose(today, src_today);
    if (!tr.isNull()) {
        result.tradesOnClose.push_back(tr);
    }
    // Aligned with runMoment: the close stage may also submit the delayed requests (e.g. a delayed
    // buy), surface them so that the upper layer (e.g. the mode A snapshot synthesis) perceives
    // the intent state
    _collectDelayRequests(result);
    return result;
}

void System::_collectDelayRequests(MomentResult& result) const {
    for (const auto& req : m_buyRequestList) {
        if (req.valid) {
            result.delayOnNextOpen.push_back(req);
        }
    }
    for (const auto& req : m_sellRequestList) {
        if (req.valid) {
            result.delayOnNextOpen.push_back(req);
        }
    }
    for (const auto& req : m_sellShortRequestList) {
        if (req.valid) {
            result.delayOnNextOpen.push_back(req);
        }
    }
    for (const auto& req : m_buyShortRequestList) {
        if (req.valid) {
            result.delayOnNextOpen.push_back(req);
        }
    }
}

TradeRecord System::_runMoment(const KRecord& today, const KRecord& src_today) {
    TradeRecord tr_open = _runMomentOnOpen(today, src_today);
    TradeRecord tr_close = _runMomentOnClose(today, src_today);
    return tr_close.isNull() ? tr_open : tr_close;
}

TradeRecord System::_runMomentOnOpen(const KRecord& today, const KRecord& src_today) {
    if (getParam<bool>("trace")) {
        HKU_INFO("{} ------------------------------------------------------", today.datetime);
        HKU_INFO(htr("[{}] cal today {}", name(), today));
        HKU_INFO_IF(m_kdata.getQuery().recoverType() != KQuery::NO_RECOVER,
                    htr("[{}] raw today {}", name(), src_today));
    }

    TradeRecord result;

    // Data error
    HKU_DEBUG_IF_RETURN((today.closePrice > today.highPrice || today.closePrice < today.lowPrice ||
                         today.lowPrice > today.highPrice),
                        result,
                        "[{}] ignore error data (close > high || close < low || low > high), date: "
                        "{}, close: {}, high: {}, low: {}",
                        name(), today.datetime, today.closePrice, today.highPrice, today.lowPrice);

    // Process the existing trade requests
    result = _processRequest(today, src_today);
    return result;
}

TradeRecord System::_runMomentOnClose(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;

    //----------------------------------------------------------
    // Process the market environment strategy
    //----------------------------------------------------------
    bool trace = getParam<bool>("trace");
    bool current_ev_valid = _environmentIsValid(today.datetime);

    // If the current environment is invalid
    if (!current_ev_valid) {
        HKU_INFO_IF(trace, htr("[{}] current EV is invalid", name()));
        TradeRecord tr;
        // If a long position is held, liquidate it; sell_delay decides the fill timing (this close
        // or the next open)
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_ENVIRONMENT);
            HKU_INFO_IF(trace, htr("[{}] EV to sell", name()));
        }
        // If a short position is held, cover it; buy_delay decides the fill timing
        else if (m_tm->haveShort(m_stock)) {
            tr = _buyShort(today, src_today, PART_ENVIRONMENT);
            HKU_INFO_IF(trace, htr("[{}] EV to buy short", name()));
        }

        m_pre_ev_valid = current_ev_valid;
        return tr.isNull() ? result : tr;
    }

    // When the environment changes from invalid to valid
    if (!m_pre_ev_valid) {
        HKU_INFO_IF(trace, htr("[{}] EV status from invalid to valid", name()));

        // If the EV is used for the initial position building
        if (getParam<bool>("ev_open_position")) {
            HKU_INFO_IF(trace, htr("[{}] EV to buy", name()));
            TradeRecord tr = _buy(today, src_today, PART_ENVIRONMENT);
            m_pre_ev_valid = current_ev_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_ev_valid = current_ev_valid;

    //----------------------------------------------------------
    // Process the system valid condition judgment strategy
    //----------------------------------------------------------

    bool current_cn_valid = _conditionIsValid(today.datetime);

    // If the system is currently invalid
    if (!current_cn_valid) {
        HKU_INFO_IF(trace, htr("[{}] current CN is invalid", name()));
        TradeRecord tr;
        // If a long position is held, liquidate it (the fill timing follows sell_delay)
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_CONDITION);
            HKU_INFO_IF(trace, htr("[{}] CN to sell", name()));
        }
        // If a short position is held, cover it (the fill timing follows buy_delay)
        else if (m_tm->haveShort(m_stock)) {
            tr = _buyShort(today, src_today, PART_CONDITION);
            HKU_INFO_IF(trace, htr("[{}] CN to buy short", name()));
        }

        m_pre_cn_valid = current_cn_valid;
        return tr.isNull() ? result : tr;
    }

    // If the system changes from invalid to valid
    if (!m_pre_cn_valid) {
        HKU_INFO_IF(trace, htr("[{}] CN status from invalid to valid", name()));

        // If the EV is used for the initial position building
        if (getParam<bool>("cn_open_position")) {
            HKU_INFO_IF(trace, htr("[{}] CN to buy", name()));
            TradeRecord tr = _buy(today, src_today, PART_CONDITION);
            m_pre_cn_valid = current_cn_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_cn_valid = current_cn_valid;

    //----------------------------------------------------------
    // Process the buy and sell signals
    //----------------------------------------------------------

    // If there is a buy signal
    if (m_sg->shouldBuy(today.datetime)) {
        TradeRecord tr;
        if (m_tm->haveShort(m_stock)) {
            HKU_INFO_IF(trace, htr("[{}] SG to buy short", name()));
            tr = _buyShort(today, src_today, PART_SIGNAL);

        } else {
            HKU_INFO_IF(trace, htr("[{}] SG to buy", name()));
            tr = _buy(today, src_today, PART_SIGNAL);
        }
        return tr.isNull() ? result : tr;
    }

    // Issue a sell signal
    if (!getParam<bool>("ignore_sell_sg") && m_sg->shouldSell(today.datetime)) {
        TradeRecord tr;
        if (m_tm->have(m_stock)) {
            HKU_INFO_IF(trace, htr("[{}] SG to sell", name()));
            tr = _sell(today, src_today, PART_SIGNAL);

        } else {
            HKU_INFO_IF(trace, htr("[{}] SG to sell short", name()));
            tr = _sellShort(today, src_today, PART_SIGNAL);
        }
        return tr.isNull() ? result : tr;
    }

    //----------------------------------------------------------
    // Process the stop-loss, take-profit and goal signals
    // The exit checks all use the original price without adjustment (the stop-loss and
    // take-profit indicators run on the adjusted KData and their results are mapped back)
    //----------------------------------------------------------

    price_t src_current_price = src_today.closePrice;  // The original price without adjustment

    PositionRecord position = m_tm->getPosition(today.datetime, m_stock);
    HKU_INFO_IF(trace, htr("[{}] current position: {}", name(), position.number));
    if (position.number != 0) {
        TradeRecord tr;
        if (src_current_price <= position.stoploss) {
            HKU_INFO_IF(trace, htr("[{}] ST to sell, current price: {}, stoploss: {}", name(),
                                   src_current_price, position.stoploss));
            tr = _sell(today, src_today, PART_STOPLOSS);

        } else if (price_t goal = _getGoalPrice(today.datetime, src_current_price);
                   src_current_price >= goal) {
            HKU_INFO_IF(trace, "[{}] {}: {}, {}: {}", name(), htr("PG to sell, current price"),
                        src_current_price, htr("goal price"), goal);
            tr = _sell(today, src_today, PART_PROFITGOAL, goal);

        } else {
            // The take-profit indicator runs on the adjusted KData, and the result is mapped back
            // to the original price coordinate so that it stays comparable across bars
            price_t current_take_profile = _getTakeProfitPrice(today, src_today, today.closePrice);
            if (current_take_profile != 0.0) {
                // The long take-profit level only rises and is never lowered
                if (getParam<bool>("tp_monotonic") && current_take_profile < m_lastTakeProfit) {
                    current_take_profile = m_lastTakeProfit;
                } else {
                    m_lastTakeProfit = current_take_profile;
                }

                int tp_delay_n = getParam<int>("tp_delay_n");
                size_t pos = m_kdata.getPos(today.datetime);
                size_t position_pos = m_kdata.getPos(position.takeDatetime);
                // The position may have been opened before the current KData window; in that case
                // the delay is counted from the window start, and the subtraction must not
                // underflow when the opening date is outside the window
                size_t bars_since_open = position_pos == Null<size_t>()
                                           ? pos + 1
                                           : (pos >= position_pos ? pos - position_pos : 0);
                // Sell when the current price is not higher than the take-profit price and the
                // take-profit delay condition is met
                price_t profit = position.number * src_today.closePrice - position.totalCost;
                if (bars_since_open >= tp_delay_n && src_current_price <= current_take_profile &&
                    profit > (position.buyMoney - position.sellMoney)) {
                    HKU_INFO_IF(trace, htr("[{}] TP to sell, current price: {}, take_profit: {}",
                                           name(), src_current_price, current_take_profile));
                    tr = _sell(today, src_today, PART_TAKEPROFIT);
                }
            }
        }

        return tr.isNull() ? result : tr;
    }

    //----------------------------------------------------------
    // Process the stop-loss, take-profit and goal signals for the short position
    //----------------------------------------------------------
    PositionRecord shortPosition = m_tm->getShortPosition(m_stock);
    HKU_INFO_IF(trace, htr("[{}] current short position: {}", name(), shortPosition.number));
    if (shortPosition.number != 0) {
        TradeRecord tr;
        if (shortPosition.stoploss != 0.0 && src_current_price >= shortPosition.stoploss) {
            HKU_INFO_IF(trace, htr("[{}] ST to buy short, current price: {}, stoploss: {}", name(),
                                   src_current_price, shortPosition.stoploss));
            tr = _buyShort(today, src_today, PART_STOPLOSS);

        } else if (price_t goal = _getShortGoalPrice(today.datetime, src_current_price);
                   src_current_price <= goal) {
            HKU_INFO_IF(trace, "[{}] {}: {}, {}: {}", name(), htr("PG to buy short, current price"),
                        src_current_price, htr("goal price"), goal);
            tr = _buyShort(today, src_today, PART_PROFITGOAL, goal);

        } else {
            // The take-profit indicator runs on the adjusted KData, and the result is mapped back
            // to the original price coordinate so that it stays comparable across bars
            price_t current_take_profile =
              _getShortTakeProfitPrice(today, src_today, today.closePrice);
            if (current_take_profile != 0.0) {
                // The short take-profit level only falls and is never raised; a zero ratchet means
                // the book was flat, so it must not clamp the new level down to 0
                if (getParam<bool>("tp_monotonic") && m_lastShortTakeProfit > 0.0 &&
                    current_take_profile > m_lastShortTakeProfit) {
                    current_take_profile = m_lastShortTakeProfit;
                } else {
                    m_lastShortTakeProfit = current_take_profile;
                }

                int tp_delay_n = getParam<int>("tp_delay_n");
                size_t pos = m_kdata.getPos(today.datetime);
                size_t position_pos = m_kdata.getPos(shortPosition.takeDatetime);
                // The position may have been opened before the current KData window; in that case
                // the delay is counted from the window start, and the subtraction must not
                // underflow when the opening date is outside the window
                size_t bars_since_open = position_pos == Null<size_t>()
                                           ? pos + 1
                                           : (pos >= position_pos ? pos - position_pos : 0);
                // buyMoney / sellMoney are gross flows and every fee accumulates in totalCost, so
                // the net condition must subtract the fees (same basis as the long side)
                price_t profit = shortPosition.sellMoney -
                                 shortPosition.number * src_today.closePrice -
                                 shortPosition.buyMoney - shortPosition.totalCost;
                // Cover when the current price is not lower than the take-profit price and the
                // take-profit delay condition is met
                if (bars_since_open >= tp_delay_n && src_current_price >= current_take_profile &&
                    profit > 0.0) {
                    HKU_INFO_IF(
                      trace, htr("[{}] TP to buy short, current price: {}, take_profit: {}", name(),
                                 src_current_price, current_take_profile));
                    tr = _buyShort(today, src_today, PART_TAKEPROFIT);
                }
            }
        }

        return tr.isNull() ? result : tr;
    }

    return result;
}

TradeRecord System::_buy(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;

    bool trace = getParam<bool>("trace");

    // A delayed buy
    if (getParam<bool>("buy_delay")) {
        _submitBuyRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will be delay to buy", name()));
        return result;
    }

    // Check whether it is a one-line limit up board
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy one-price board", name()));
            return _buyNow(today, src_today, from);
        }

        // Get yesterday's close price and check whether it is a one-line limit up
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", name()));
            _submitBuyRequest(today, src_today, from);
            return result;
        }

        const auto& pre_day = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > pre_day.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price up-limit board", name()));
            _submitBuyRequest(today, src_today, from);
            return result;
        }
    }

    // Delay the trade when the volume and the turnover amount are 0
    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace, htr("[{}] delay to buy, current amount == 0 or count == 0", name()));
        _submitBuyRequest(today, src_today, from);
        return result;
    }

    return _buyNow(today, src_today, from);
}

TradeRecord System::_buyNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;

    // Take the current close price as the planned price
    price_t planPrice = src_today.closePrice;

    // The price actually filled: the slippage result snapped to the market tick, which can differ
    // from the plan price, so the checks and the size below must use it
    price_t realPrice = _getRealBuyPrice(src_today, planPrice);
    HKU_WARN_IF_RETURN(!(realPrice > 0.0), result, "[{}] {}: invalid buy price!", name(),
                       today.datetime);

    // Calculate the stop-loss price
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // Give up the trade when the fill price is not higher than the stop-loss price; an invalid
    // level has already been turned into 0 (no stop-loss) by _getStoplossPrice
    bool trace = getParam<bool>("trace");
    if (realPrice <= stoploss) {
        HKU_INFO_IF(trace,
                    htr("[{}] buy failed, price: {} <= stoploss: {}", name(), realPrice, stoploss));
        return result;
    }

    // Get the buyable quantity at the fill price: the cash clamp of the money manager is then
    // exact, because the trade manager charges that same price
    double number = _getBuyNumber(today.datetime, realPrice, realPrice - stoploss, from);
    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    // Guard the int64 cast below: a NaN/inf or out-of-range raw number makes it undefined
    // behaviour, so the upper bound must be checked before the truncation
    if (!std::isfinite(number) || number <= 0.0 || number > m_stock.maxTradeNumber()) {
        HKU_INFO_IF(trace, "[{}] {}, invalid number: {}", name(), htr("buy failed"), number);
        return result;
    }
    number = int64_t(number / min_num) * min_num;
    if (number == 0.0) {
        HKU_INFO_IF(trace, "[{}] {}, number < minTradeNumber: {}", name(), htr("buy failed"),
                    min_num);
        return result;
    }

    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    _autoCheckin(today.datetime, realPrice, number);
    TradeRecord record =
      m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_BUY != record.business) {
        HKU_INFO_IF(trace, htr("[{}] buy failed, {}", name(), record));
        return result;
    }

    // The long take-profit ratchet only rises, so an add-on at a lower price keeps the old level
    m_lastTakeProfit = std::max(m_lastTakeProfit, record.realPrice);
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    return record;
}

TradeRecord System::_buyDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");

    if (m_buyRequestList.empty()) {
        return result;
    }

    // Delay the trade when the volume and the turnover amount are 0
    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace, htr("[{}] delay to buy, current amount == 0 or count == 0", name()));
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's close price and check whether it is a one-line limit up
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", name()));
            return result;
        }

        const auto& pre_day = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > pre_day.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price up-limit board", name()));
            return result;
        }
    }

    // Process all the delayed requests
    while (!m_buyRequestList.empty()) {
        TradeRequest& req = m_buyRequestList.front();

        // Check whether the maximum delay count is exceeded
        if (req.count > getParam<int>("max_delay_count")) {
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }
        req.count++;

        // A delayed operation, take the open price of the current moment
        price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

        // The price actually filled, see _buyNow: the size and the checks below must use it
        price_t realPrice = _getRealBuyPrice(src_today, planPrice);
        if (!(realPrice > 0.0)) {
            HKU_WARN("[{}] {}: invalid delay buy price!", name(), today.datetime);
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }

        // Calculate the stop-loss price and the buyable quantity
        price_t stoploss = 0.0;
        double number = 0.0;
        price_t goalPrice = 0.0;
        if (getParam<bool>("delay_use_current_price")) {
            // Calculate the stop-loss price and the buyable quantity with the current planned price
            stoploss = _getStoplossPrice(today, src_today, today.openPrice);
            number = realPrice <= stoploss
                       ? 0.0
                       : _getBuyNumber(today.datetime, realPrice, realPrice - stoploss, req.from);
            goalPrice = _getGoalPrice(today.datetime, planPrice);

        } else {
            stoploss = req.stoploss;
            number = req.number;
            goalPrice = req.goal;

            // The quantity was decided at the signal bar's close while the fill happens at this
            // bar's open: when the fill price no longer fits the cash, re-size at the fill price
            // instead of letting the trade manager reject the whole stale order
            if (number > 0.0 && m_mm && !m_mm->getParam<bool>("auto_checkin")) {
                int precision = m_tm->precision();
                CostRecord cost = m_tm->getBuyCost(today.datetime, m_stock, realPrice, number);
                price_t need = roundEx(
                  roundEx(realPrice * number * m_stock.unit(), precision) + cost.total, precision);
                if (m_tm->cash(today.datetime, m_kdata.getQuery().kType()) < need) {
                    number =
                      _getBuyNumber(today.datetime, realPrice, realPrice - stoploss, req.from);
                }
            }
        }

        // Drop the request when the fill price is not higher than the stop-loss price or when the
        // buy quantity is 0
        if (realPrice <= stoploss || number <= 0) {
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }

        double min_num = m_stock.minTradeNumber();
        HKU_ASSERT(min_num != 0.0);
        // Guard the int64 cast below: a NaN/inf or out-of-range raw number makes it undefined
        // behaviour, so the upper bound must be checked before the truncation
        if (!std::isfinite(number) || number > m_stock.maxTradeNumber()) {
            HKU_INFO_IF(trace, "[{}] {}, invalid number: {}", name(), htr("delay buy failed"),
                        number);
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }
        number = int64_t(number / min_num) * min_num;
        if (number <= 0.0) {
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }

        _autoCheckin(today.datetime, realPrice, number);
        TradeRecord record = m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss,
                                       goalPrice, planPrice, req.from);
        if (BUSINESS_BUY != record.business) {
            m_buyRequestList.erase(m_buyRequestList.begin());
            continue;
        }

        m_lastTakeProfit = std::max(m_lastTakeProfit, record.realPrice);
        m_trade_list.push_back(record);
        _buyNotifyAll(record);
        m_buyRequestList.erase(m_buyRequestList.begin());
        result = record;
    }

    return result;
}

void System::_submitBuyRequest(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRequest req;
    req.valid = true;
    req.business = BUSINESS_BUY;
    req.from = from;
    req.count = 0;
    req.datetime = today.datetime;
    req.stoploss = _getStoplossPrice(today, src_today, today.closePrice);
    req.goal = _getGoalPrice(today.datetime, src_today.closePrice);
    req.number = _getBuyNumber(today.datetime, src_today.closePrice,
                               src_today.closePrice - req.stoploss, from);
    m_buyRequestList.push_back(req);
}

void System::_autoCheckin(const Datetime& datetime, price_t realPrice, double number) {
    HKU_IF_RETURN(!m_mm->getParam<bool>("auto_checkin"), void());
    HKU_IF_RETURN(datetime < m_tm->lastDatetime(), void());

    // Deposit once per trade with the affordability formula of TradeManager::buy; the gap is
    // rounded up so the following buy always passes its cash check
    int precision = m_tm->precision();
    CostRecord cost = m_tm->getBuyCost(datetime, m_stock, realPrice, number);
    price_t needed =
      roundEx(roundEx(realPrice * number * m_stock.unit(), precision) + cost.total, precision);
    price_t cash = m_tm->cash(datetime, m_kdata.getQuery().kType());
    if (cash < needed) {
        m_tm->checkin(datetime, roundUp(needed - cash, precision));
    }
}

TradeRecord System::_sellForce(const Datetime& date, double num, Part from, bool on_open) {
    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[{}] {} {} by {}", name(), htr("force sell"), num, getSystemPartName(from));

    TradeRecord record;
    size_t pos = m_kdata.getPos(date);
    HKU_TRACE_IF_RETURN(pos == Null<size_t>(), record,
                        "Failed to sellForce {}, the day {} could'nt sell!", m_stock.market_code(),
                        date);

    PositionRecord position = m_tm->getPosition(date, m_stock);
    // A short-only book (the mode C shared account) has no long position: the force-flatten covers
    // the short instead of selling
    const bool cover_short = (position.number <= 0.0);
    if (cover_short) {
        position = m_tm->getShortPosition(m_stock);
    }
    HKU_IF_RETURN(position.number <= 0.0, record);

    const auto& src_krecord =
      m_stock.getKRecord(m_kdata.startPos() + pos, m_kdata.getQuery().kType());

    price_t plan_price = on_open ? src_krecord.openPrice : src_krecord.closePrice;
    // Covering a short is a buy, so it must take the short-side buy price, not the sell price
    price_t realPrice = cover_short ? _getRealBuyShortPrice(src_krecord, plan_price)
                                    : _getRealSellPrice(src_krecord, plan_price);
    // A stale out-of-range src_krecord yields NullKRecord whose price is 0.0; trading at 0 would
    // clear the position and evaporate the funds, so reject it here
    HKU_WARN_IF_RETURN(realPrice <= 0.0, record, "[{}] {}: invalid force price!",
                       m_stock.market_code(), date);

    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    // MAX_DOUBLE is the "sell everything" sentinel of the quantity and must be resolved before the
    // board-lot rounding below: the double -> int64 conversion of an out-of-range value is
    // undefined behavior, and its result differs between compilers and platforms (it used to make
    // the sell fail on MSVC while selling the whole position on clang/arm64)
    double real_sell_num;
    if (num >= MAX_DOUBLE || num >= position.number) {
        real_sell_num = position.number;
    } else {
        // Round the quantity to be sold to an integer multiple of the minimum trade unit; when the
        // remainder is less than the minimum trade unit, sell everything at once
        real_sell_num = static_cast<int64_t>(num / min_num) * min_num;
        if (position.number - real_sell_num < min_num) {
            real_sell_num = position.number;
        }
    }

    record = cover_short ? m_tm->buyShort(date, m_stock, realPrice, real_sell_num,
                                          position.stoploss, position.goalPrice, plan_price, from)
                         : m_tm->sell(date, m_stock, realPrice, real_sell_num, position.stoploss,
                                      position.goalPrice, plan_price, from);
    HKU_WARN_IF_RETURN(record == Null<TradeRecord>(), record, "[{}] {}: {} by {}", name(),
                       htr("Failed force sell"), num, getSystemPartName(from));

    // The take-profit ratchet is initialized to 0 when its book is flat; the forced flattening
    // covers whichever side the account holds, so both sides are reset here
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    }
    if (!m_tm->haveShort(m_stock)) {
        m_lastShortTakeProfit = 0.0;
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    return record;
}

TradeRecord System::_sell(const KRecord& today, const KRecord& src_today, Part from, price_t goal) {
    bool trace = getParam<bool>("trace");
    TradeRecord result;
    if (getParam<bool>("sell_delay")) {
        _submitSellRequest(today, src_today, from, goal);
        HKU_INFO_IF(trace, htr("[{}] will be delay to sell", name()));
        return result;
    }

    // Check whether it may be a one-line limit down
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] sell one-price board", name()));
            return _sellNow(today, src_today, from, goal);
        }

        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name()));
            _submitSellRequest(today, src_today, from, goal);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name()));
            _submitSellRequest(today, src_today, from, goal);
            return result;
        }
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", name()));
        _submitSellRequest(today, src_today, from, goal);
        return result;
    }

    result = _sellNow(today, src_today, from, goal);
    HKU_INFO_IF(trace, htr("[{}] sell now: {}", name(), result));
    return result;
}

TradeRecord System::_sellNow(const KRecord& today, const KRecord& src_today, Part from,
                             price_t goal) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;
    double number = 0;

    // The price actually filled, which can differ from the plan price
    price_t realPrice = _getRealSellPrice(src_today, planPrice);
    HKU_WARN_IF_RETURN(!(realPrice > 0.0), result, "[{}] {}: invalid sell price!", name(),
                       today.datetime);

    // Calculate the new stop-loss price
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // When the fill price is not higher than the new stop-loss price, the whole position is to be
    // sold
    number = _getSellNumber(today.datetime, realPrice, realPrice - stoploss, from);
    if (number <= 0) {
        return result;
    }

    price_t goalPrice = std::isfinite(goal) ? goal : _getGoalPrice(today.datetime, planPrice);
    TradeRecord record =
      m_tm->sell(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_SELL != record.business) {
        return result;  // The sell operation failed
    }

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    } else {
        m_lastTakeProfit = src_today.closePrice;
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    return record;
}

TradeRecord System::_sellDelay(const KRecord& today, const KRecord& src_today) {
    bool trace = getParam<bool>("trace");
    TradeRecord result;

    if (m_sellRequestList.empty()) {
        return result;
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", name()));
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name()));
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name()));
            return result;
        }
    }

    while (!m_sellRequestList.empty()) {
        TradeRequest& req = m_sellRequestList.front();

        if (req.count > getParam<int>("max_delay_count")) {
            m_sellRequestList.erase(m_sellRequestList.begin());
            continue;
        }
        req.count++;

        price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

        // The stop-loss price at the moment the sell request is issued
        price_t stoploss = 0.0;
        double number = 0.0;
        price_t goalPrice = 0.0;

        // The price actually filled, which can differ from the plan price
        price_t realPrice = _getRealSellPrice(src_today, planPrice);
        if (!(realPrice > 0.0)) {
            HKU_WARN("[{}] {}: invalid delay sell price!", name(), today.datetime);
            m_sellRequestList.erase(m_sellRequestList.begin());
            continue;
        }

        if (getParam<bool>("delay_use_current_price")) {
            stoploss = _getStoplossPrice(today, src_today, today.openPrice);
            number = _getSellNumber(today.datetime, realPrice, realPrice - stoploss, req.from);
            goalPrice = _getGoalPrice(today.datetime, planPrice);
        } else {
            stoploss = req.stoploss;
            number = req.number;
            goalPrice = req.goal;

            // Never ask for more than the current holding: the position may have changed since
            // the signal bar, and the trade manager rejects such a sell outright
            number = std::min(number, m_tm->getHoldNumber(today.datetime, m_stock));
        }

        if (number <= 0) {
            m_sellRequestList.erase(m_sellRequestList.begin());
            continue;
        }
        TradeRecord record = m_tm->sell(today.datetime, m_stock, realPrice, number, stoploss,
                                        goalPrice, planPrice, req.from);
        if (BUSINESS_SELL != record.business) {
            m_sellRequestList.erase(m_sellRequestList.begin());
            continue;  // The sell operation failed
        }

        // The last take-profit price is initialized to 0 when there is no position
        if (!m_tm->have(m_stock)) {
            m_lastTakeProfit = 0.0;
        } else {
            m_lastTakeProfit = src_today.openPrice;
        }

        m_trade_list.push_back(record);
        _sellNotifyAll(record);
        m_sellRequestList.erase(m_sellRequestList.begin());
        result = record;
    }

    return result;
}

void System::_submitSellRequest(const KRecord& today, const KRecord& src_today, Part from,
                                price_t goal) {
    TradeRequest req;
    req.valid = true;
    req.business = BUSINESS_SELL;
    req.count = 0;
    req.from = from;
    req.datetime = today.datetime;
    req.stoploss = _getStoplossPrice(today, src_today, today.closePrice);
    if (src_today.closePrice <= req.stoploss) {
        req.number = m_tm->getHoldNumber(today.datetime, m_stock);
    } else {
        req.number = _getSellNumber(today.datetime, src_today.closePrice,
                                    src_today.closePrice - req.stoploss, from);
    }
    req.goal = std::isfinite(goal) ? goal : _getGoalPrice(today.datetime, src_today.closePrice);
    m_sellRequestList.push_back(req);
}

TradeRecord System::_buyShort(const KRecord& today, const KRecord& src_today, Part from,
                              price_t goal) {
    TradeRecord result;
    if (getParam<bool>("support_borrow_stock") == false)
        return result;

    bool trace = getParam<bool>("trace");
    if (getParam<bool>("buy_delay")) {
        _submitBuyShortRequest(today, src_today, from, goal);
        HKU_INFO_IF(trace, htr("[{}] will buy short next bar open", name()));
        return result;
    }

    // The one-line board case
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy short one-price board", name()));
            return _buyShortNow(today, src_today, from, goal);
        }

        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", name()));
            _submitBuyShortRequest(today, src_today, from, goal);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", name()));
            _submitBuyShortRequest(today, src_today, from, goal);
            return result;
        }
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", name()));
        _submitBuyShortRequest(today, src_today, from, goal);
        return result;
    }

    return _buyShortNow(today, src_today, from, goal);
}

TradeRecord System::_buyShortNow(const KRecord& today, const KRecord& src_today, Part from,
                                 price_t goal) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;  // Take the close price of the current moment

    // The price actually filled, which can differ from the plan price
    price_t realPrice = _getRealBuyShortPrice(src_today, planPrice);
    if (!(realPrice > 0.0)) {
        HKU_WARN("[{}] {}: invalid short cover price!", name(), today.datetime);
        m_buyShortRequestList.clear();
        return result;
    }

    // Take the stop-loss price corresponding to the close price of the current moment
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    // Determine the quantity
    double number = _getBuyShortNumber(today.datetime, realPrice, stoploss - realPrice, from);
    if (number <= 0) {
        m_buyShortRequestList.clear();
        return result;
    }

    // Get the holding status of the current short position
    PositionRecord pos = m_tm->getShortPosition(m_stock);
    if (pos.number <= 0) {
        m_buyShortRequestList.clear();
        return result;
    }

    if (number > pos.number) {
        number = pos.number;
    }

    price_t goalPrice = std::isfinite(goal) ? goal : _getShortGoalPrice(today.datetime, planPrice);

    TradeRecord record = m_tm->buyShort(today.datetime, m_stock, realPrice, number, stoploss,
                                        goalPrice, planPrice, from);
    if (BUSINESS_BUY_SHORT != record.business) {
        m_buyShortRequestList.clear();
        return result;
    }

    // The last short take-profit price is initialized to 0 when there is no short position
    if (!m_tm->haveShort(m_stock)) {
        m_lastShortTakeProfit = 0.0;
    } else {
        m_lastShortTakeProfit = src_today.closePrice;
    }
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequestList.clear();
    return record;
}

TradeRecord System::_buyShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");

    if (m_buyShortRequestList.empty()) {
        return result;
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", name()));
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", name()));
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", name()));
            return result;
        }
    }

    while (!m_buyShortRequestList.empty()) {
        TradeRequest& req = m_buyShortRequestList.front();

        if (req.count > getParam<int>("max_delay_count")) {
            m_buyShortRequestList.erase(m_buyShortRequestList.begin());
            continue;
        }
        req.count++;

        price_t planPrice = src_today.openPrice;  // Take the close price of the current moment

        price_t stoploss = 0.0;
        double number = 0.0;
        price_t goalPrice = 0.0;

        // The price actually filled, which can differ from the plan price
        price_t realPrice = _getRealBuyShortPrice(src_today, planPrice);
        if (!(realPrice > 0.0)) {
            HKU_WARN("[{}] {}: invalid delay short cover price!", name(), today.datetime);
            m_buyShortRequestList.erase(m_buyShortRequestList.begin());
            continue;
        }

        if (getParam<bool>("delay_use_current_price")) {
            // Take the stop-loss price corresponding to the close price of the current moment
            stoploss = _getShortStoplossPrice(today, src_today, today.openPrice);
            number = _getBuyShortNumber(today.datetime, realPrice, stoploss - realPrice, req.from);
            goalPrice = _getShortGoalPrice(today.datetime, planPrice);

        } else {
            stoploss = req.stoploss;
            number = req.number;
            goalPrice = req.goal;
        }

        if (number <= 0) {
            m_buyShortRequestList.erase(m_buyShortRequestList.begin());
            continue;
        }

        // Get the holding status of the current short position
        PositionRecord pos = m_tm->getShortPosition(m_stock);
        if (pos.number <= 0) {
            m_buyShortRequestList.erase(m_buyShortRequestList.begin());
            continue;
        }

        if (number > pos.number) {
            number = pos.number;
        }

        TradeRecord record = m_tm->buyShort(today.datetime, m_stock, realPrice, number, stoploss,
                                            goalPrice, planPrice, req.from);
        if (BUSINESS_BUY_SHORT != record.business) {
            m_buyShortRequestList.erase(m_buyShortRequestList.begin());
            continue;
        }

        // The last short take-profit price is initialized to 0 when there is no short position
        if (!m_tm->haveShort(m_stock)) {
            m_lastShortTakeProfit = 0.0;
        } else {
            m_lastShortTakeProfit = src_today.openPrice;
        }
        m_trade_list.push_back(record);
        _buyNotifyAll(record);
        m_buyShortRequestList.erase(m_buyShortRequestList.begin());
        result = record;
    }

    return result;
}

void System::_submitBuyShortRequest(const KRecord& today, const KRecord& src_today, Part from,
                                    price_t goal) {
    TradeRequest req;
    req.valid = true;
    req.business = BUSINESS_BUY_SHORT;
    req.from = from;
    req.count = 0;
    req.datetime = today.datetime;
    req.stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);
    req.goal =
      std::isfinite(goal) ? goal : _getShortGoalPrice(today.datetime, src_today.closePrice);
    req.number = _getBuyShortNumber(today.datetime, src_today.closePrice,
                                    req.stoploss - src_today.closePrice, from);
    m_buyShortRequestList.push_back(req);
}

TradeRecord System::_sellShort(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (getParam<bool>("support_borrow_stock") == false) {
        // HKU_WARN("set system param support_borrow_stock to true to short sell");
        return result;
    }

    if (getParam<bool>("sell_delay")) {
        _submitSellShortRequest(today, src_today, from);
        return result;
    }

    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy short one-price board", name()));
            return _sellShortNow(today, src_today, from);
        }

        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell short, one-price board", name()));
            _submitSellShortRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short selling delayed: limit-down lock", name()));
            _submitSellShortRequest(today, src_today, from);
            return result;
        }
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to sell short, current amount == 0 or count == 0", name()));
        _submitSellShortRequest(today, src_today, from);
        return result;
    }

    return _sellShortNow(today, src_today, from);
}

TradeRecord System::_sellShortNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // It cannot be sold at the moment, delay the sell to the next moment
        _submitSellShortRequest(today, src_today, from);
        return result;
    }

    price_t planPrice = src_today.closePrice;

    // The price actually filled, which drives the size, the risk and the check below
    price_t realPrice = _getRealSellShortPrice(src_today, planPrice);
    if (!(realPrice > 0.0)) {
        HKU_WARN("[{}] {}: invalid sell short price!", name(), today.datetime);
        m_sellShortRequestList.clear();
        return result;
    }

    // Calculate the stop-loss price
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    // Give up the trade when the short stop-loss is not above the fill price (the cover would
    // trigger immediately, symmetric with the long-side price <= stoploss check)
    bool trace = getParam<bool>("trace");
    if (stoploss > 0.0 && stoploss <= realPrice) {
        HKU_INFO_IF(trace, "[{}] sell short failed, stoploss: {} <= price: {}", name(), stoploss,
                    realPrice);
        m_sellShortRequestList.clear();
        return result;
    }

    double number =
      _getSellShortNumber(today.datetime, realPrice, _getShortOpenRisk(stoploss, realPrice), from);
    if (number <= 0) {
        m_sellShortRequestList.clear();
        return result;
    }

    price_t goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                         goalPrice, planPrice, from);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequestList.clear();
        return result;  // The sell operation failed
    }

    // The short take-profit ratchet only falls, so an add-on at a higher price keeps the old level;
    // 0 means the book was flat
    m_lastShortTakeProfit = m_lastShortTakeProfit == 0.0 || realPrice < m_lastShortTakeProfit
                              ? realPrice
                              : m_lastShortTakeProfit;
    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    m_sellShortRequestList.clear();
    return record;
}

TradeRecord System::_sellShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");

    if (m_sellShortRequestList.empty()) {
        return result;
    }

    if (today.transAmount == 0.0 || today.transCount == 0.0) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to sell short, current amount == 0 or count == 0", name()));
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell short, one-price board", name()));
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short selling delayed: limit-down lock", name()));
            return result;
        }
    }

    while (!m_sellShortRequestList.empty()) {
        TradeRequest& req = m_sellShortRequestList.front();

        if (req.count > getParam<int>("max_delay_count")) {
            m_sellShortRequestList.erase(m_sellShortRequestList.begin());
            continue;
        }
        req.count++;

        price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

        // The stop-loss price at the moment the sell request is issued
        price_t stoploss = 0.0;
        double number = 0;
        price_t goalPrice = 0.0;

        // The price actually filled, which drives the size and the check below
        price_t realPrice = _getRealSellShortPrice(src_today, planPrice);
        if (!(realPrice > 0.0)) {
            HKU_WARN("[{}] {}: invalid delay sell short price!", name(), today.datetime);
            m_sellShortRequestList.erase(m_sellShortRequestList.begin());
            continue;
        }

        if (getParam<bool>("delay_use_current_price")) {
            stoploss = _getShortStoplossPrice(today, src_today, today.openPrice);
            number = _getSellShortNumber(today.datetime, realPrice,
                                         _getShortOpenRisk(stoploss, realPrice), req.from);
            goalPrice = _getShortGoalPrice(today.datetime, planPrice);
        } else {
            stoploss = req.stoploss;
            number = req.number;
            goalPrice = req.goal;
        }

        // Drop the request when the short stop-loss is not above the fill price, symmetric with
        // the long-side price <= stoploss check
        if ((stoploss > 0.0 && stoploss <= realPrice) || number <= 0) {
            m_sellShortRequestList.erase(m_sellShortRequestList.begin());
            continue;
        }

        TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                             goalPrice, planPrice, req.from);
        if (BUSINESS_SELL_SHORT != record.business) {
            m_sellShortRequestList.erase(m_sellShortRequestList.begin());
            continue;  // The sell operation failed
        }

        m_lastShortTakeProfit = m_lastShortTakeProfit == 0.0 || realPrice < m_lastShortTakeProfit
                                  ? realPrice
                                  : m_lastShortTakeProfit;
        m_trade_list.push_back(record);
        _sellNotifyAll(record);
        m_sellShortRequestList.erase(m_sellShortRequestList.begin());
        result = record;
    }

    return result;
}

void System::_submitSellShortRequest(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRequest req;
    req.valid = true;
    req.business = BUSINESS_SELL_SHORT;
    req.from = from;
    req.count = 0;
    req.datetime = today.datetime;
    req.stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);
    req.goal = _getShortGoalPrice(today.datetime, src_today.closePrice);
    req.number = _getSellShortNumber(today.datetime, src_today.closePrice,
                                     _getShortOpenRisk(req.stoploss, src_today.closePrice), from);
    m_sellShortRequestList.push_back(req);
}

TradeRecord System::_processRequest(const KRecord& today, const KRecord& src_today) {
    // One queue per bar: the cash generating side (sell / sell short) is served before the cash
    // consuming side, so a pending sell frees the funds a pending buy is waiting for
    HKU_IF_RETURN(!m_sellRequestList.empty(), _sellDelay(today, src_today));
    HKU_IF_RETURN(!m_sellShortRequestList.empty(), _sellShortDelay(today, src_today));
    HKU_IF_RETURN(!m_buyRequestList.empty(), _buyDelay(today, src_today));
    HKU_IF_RETURN(!m_buyShortRequestList.empty(), _buyShortDelay(today, src_today));
    return TradeRecord();
}

TradeRecord System::pfProcessDelaySellRequest(const Datetime& date) {
    HKU_IF_RETURN(m_sellRequestList.empty(), TradeRecord());
    size_t pos = m_kdata.getPos(date);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _sellDelay(today, src_today);
}

TradeRecord System::pfProcessDelayBuyRequest(const Datetime& date) {
    HKU_IF_RETURN(m_buyRequestList.empty(), TradeRecord());
    size_t pos = m_kdata.getPos(date);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _buyDelay(today, src_today);
}

price_t System::_getStoplossPrice(const KRecord& today, const KRecord& src_today, price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    price_t level = m_st->getPrice(today.datetime, price);
    // Null (non-finite) means the strategy returned no usable level: the entry still takes place
    // without stop-loss (sized with the full planned price as risk) instead of being dropped
    if (!std::isfinite(level)) {
        HKU_INFO_IF(getParam<bool>("trace"), "[{}] invalid stoploss, open without stop-loss",
                    name());
        return 0.0;
    }
    // Zero means no stop-loss on this bar and must not be turned into a real level by the mapping
    // (e.g. into the low price of a one-price board)
    HKU_IF_RETURN(level == 0.0, 0.0);
    return mapAdjustedToOriginal(today, src_today, level);
}

price_t System::_getShortStoplossPrice(const KRecord& today, const KRecord& src_today,
                                       price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    price_t level = m_st->getShortPrice(today.datetime, price);
    // Null (non-finite) means no usable level: the short entry still takes place without stop-loss
    if (!std::isfinite(level)) {
        HKU_INFO_IF(getParam<bool>("trace"), "[{}] invalid stoploss, open short without stop-loss",
                    name());
        return 0.0;
    }
    HKU_IF_RETURN(level == 0.0, 0.0);
    return mapAdjustedToOriginal(today, src_today, level);
}

price_t System::_getTakeProfitPrice(const KRecord& today, const KRecord& src_today, price_t price) {
    HKU_IF_RETURN(!m_tp, 0.0);
    price_t level = m_tp->getPrice(today.datetime, price);
    // Null means the strategy returned an invalid level: no take-profit on this bar
    HKU_IF_RETURN(!std::isfinite(level), 0.0);
    // Zero means no take-profit price on this bar and must not be turned into a real level by
    // the mapping (e.g. into the low price of a one-price board)
    HKU_IF_RETURN(level == 0.0, 0.0);
    return mapAdjustedToOriginal(today, src_today, level);
}

price_t System::_getShortTakeProfitPrice(const KRecord& today, const KRecord& src_today,
                                         price_t price) {
    HKU_IF_RETURN(!m_tp, 0.0);
    price_t level = m_tp->getShortPrice(today.datetime, price);
    HKU_IF_RETURN(!std::isfinite(level), 0.0);
    HKU_IF_RETURN(level == 0.0, 0.0);
    return mapAdjustedToOriginal(today, src_today, level);
}

// The slippage result is the last price seen before the trade manager, which accepts 0 as the
// market-order placeholder, so an invalid value must never be passed through; and a fill outside
// the current bar's range is impossible, so both sides are clamped. The original (unadjusted)
// record of the same bar is already at hand and shares the coordinate of the plan price.
// A stale or zero-price original record still yields a plan price of 0, which is why the position
// closing entries reject a non-positive real price: a forced liquidation asks the money manager for
// the whole position before it ever looks at the price, so nothing else stops a zero-price fill.
static price_t fixupSlippagePrice(const string& sys_name, const Stock& stock,
                                  const KRecord& src_today, price_t planPrice, price_t price,
                                  const char* side) {
    if (!std::isfinite(price) || price <= 0.0) {
        HKU_DEBUG("[{}] invalid slippage {} price({}), fallback to the plan price({})", sys_name,
                  side, price, planPrice);
        price = planPrice;
    } else {
        // The model price is snapped to the market's minimum price tick; the plan price and the bar
        // bounds are raw market prices, already tick aligned, and are used as they are
        price = roundEx(price, stock.isNull() ? 2 : stock.precision());
    }
    if (src_today.isValid() && (price < src_today.lowPrice || price > src_today.highPrice)) {
        HKU_DEBUG("[{}] slippage {} price({}) is out of the bar range [{}, {}], clamp it", sys_name,
                  side, price, src_today.lowPrice, src_today.highPrice);
        price = std::min(std::max(price, src_today.lowPrice), src_today.highPrice);
    }
    return price;
}

price_t System::_getRealBuyPrice(const KRecord& src_today, price_t planPrice) {
    price_t price = m_sp ? m_sp->getRealBuyPrice(src_today.datetime, planPrice) : planPrice;
    return fixupSlippagePrice(name(), m_stock, src_today, planPrice, price, "buy");
}

price_t System::_getRealSellPrice(const KRecord& src_today, price_t planPrice) {
    price_t price = m_sp ? m_sp->getRealSellPrice(src_today.datetime, planPrice) : planPrice;
    return fixupSlippagePrice(name(), m_stock, src_today, planPrice, price, "sell");
}

price_t System::_getRealBuyShortPrice(const KRecord& src_today, price_t planPrice) {
    price_t price = m_sp ? m_sp->getRealBuyShortPrice(src_today.datetime, planPrice) : planPrice;
    return fixupSlippagePrice(name(), m_stock, src_today, planPrice, price, "buy short");
}

price_t System::_getRealSellShortPrice(const KRecord& src_today, price_t planPrice) {
    price_t price = m_sp ? m_sp->getRealSellShortPrice(src_today.datetime, planPrice) : planPrice;
    return fixupSlippagePrice(name(), m_stock, src_today, planPrice, price, "sell short");
}

json System::lastSuggestion() const {
    json result;
    json sys_json;
    sys_json["name"] = name();
    if (m_stock.isNull()) {
        sys_json["stock"] = nullptr;
    } else {
        sys_json["stock"] = m_stock.market_code();
    }

    if (!m_tm) {
        sys_json["suggestion"] = nullptr;
        result["sys"] = sys_json;
        return result;
    }

    Datetime tm_lastdatetime = m_tm->lastDatetime();
    Datetime kdata_lastdatetime = m_kdata.empty() ? Null<Datetime>() : m_kdata.back().datetime;

    json suggestion;
    if (tm_lastdatetime == kdata_lastdatetime) {
        const TradeRecordList& tr_list = m_tm->getRefTradeList();
        json on_last_close = json::array();
        for (const auto& record : tr_list) {
            if (m_stock == record.stock && record.datetime == kdata_lastdatetime) {
                json rec;
                rec["stock"] = record.stock.market_code();
                rec["datetime"] = record.datetime.str();
                rec["business"] = getBusinessName(record.business);
                rec["planPrice"] = record.planPrice;
                rec["realPrice"] = record.realPrice;
                rec["goalPrice"] = record.goalPrice;
                rec["number"] = record.number;
                rec["stoploss"] = record.stoploss;
                rec["cash"] = record.cash;
                rec["from"] = record.from;
                rec["remark"] = record.remark;

                // Add the cost field
                json cost;
                cost["commission"] = record.cost.commission;
                cost["stamptax"] = record.cost.stamptax;
                cost["transferfee"] = record.cost.transferfee;
                cost["others"] = record.cost.others;
                cost["total"] = record.cost.total;
                rec["cost"] = cost;

                on_last_close.push_back(rec);
            }
        }
        suggestion["last_trade_record"] = on_last_close;
    } else {
        suggestion["last_trade_record"] = nullptr;
    }

    json delay_on_next_open = json::array();
    for (const auto& req : m_buyRequestList) {
        json buy_request;
        buy_request["stock"] = m_stock.market_code();
        buy_request["business"] = getBusinessName(req.business);
        buy_request["datetime"] = req.datetime.str();
        buy_request["stoploss"] = req.stoploss;
        buy_request["goal"] = req.goal;
        buy_request["number"] = req.number;
        buy_request["from"] = getSystemPartName(req.from);
        buy_request["remark"] = req.remark;
        buy_request["count"] = req.count;
        delay_on_next_open.push_back(buy_request);
    }

    for (const auto& req : m_sellRequestList) {
        json sell_request;
        sell_request["stock"] = m_stock.market_code();
        sell_request["business"] = getBusinessName(req.business);
        sell_request["datetime"] = req.datetime.str();
        sell_request["stoploss"] = req.stoploss;
        sell_request["goal"] = req.goal;
        sell_request["number"] = req.number;
        sell_request["from"] = getSystemPartName(req.from);
        sell_request["remark"] = req.remark;
        sell_request["count"] = req.count;
        delay_on_next_open.push_back(sell_request);
    }

    if (!delay_on_next_open.empty()) {
        suggestion["delay_on_next_open"] = delay_on_next_open;
    } else {
        suggestion["delay_on_next_open"] = nullptr;
    }

    sys_json["suggestion"] = suggestion;
    result["sys"] = sys_json;
    return result;
}

} /* namespace hku */
