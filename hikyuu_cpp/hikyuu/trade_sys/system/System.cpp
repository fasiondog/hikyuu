/*
 * SystemBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "hikyuu/global/sysinfo.h"
#include "System.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::System)
#endif

namespace hku {

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
  m_buy_days(0),
  m_sell_short_days(0),
  m_lastTakeProfit(0.0),
  m_lastShortTakeProfit(0.0) {
    initParam();
}

System::System(const string& name)
: m_name(name),
  m_calculated(false),
  m_pre_ev_valid(true),
  m_pre_cn_valid(true),
  m_buy_days(0),
  m_sell_short_days(0),
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
  m_buy_days(0),
  m_sell_short_days(0),
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

    // Whether to use the market environment for the initial position building
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

    m_buy_days = 0;
    m_sell_short_days = 0;
    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();

    _reset();
}

void System::forceResetAll() {
    if (m_tm)
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

    m_buy_days = 0;
    m_sell_short_days = 0;
    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();

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

SystemPtr System::clone() {
    SystemPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = SystemPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

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

    p->m_calculated = m_calculated;
    p->m_pre_ev_valid = m_pre_ev_valid;
    p->m_pre_cn_valid = m_pre_cn_valid;

    p->m_buy_days = m_buy_days;
    p->m_sell_short_days = m_sell_short_days;
    p->m_trade_list = m_trade_list;
    p->m_lastTakeProfit = m_lastTakeProfit;
    p->m_lastShortTakeProfit = m_lastShortTakeProfit;

    p->m_buyRequest = m_buyRequest;
    p->m_sellRequest = m_sellRequest;
    p->m_sellShortRequest = m_sellShortRequest;
    p->m_buyShortRequest = m_buyShortRequest;

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

    // When a market environment strategy exists, the default previous-day market valid
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
    m_buyRequest.clear();
}

TradeRecord System::runMoment(const Datetime& datetime) {
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _runMoment(today, src_today);
}

TradeRecord System::runMomentOnOpen(const Datetime& datetime) {
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _runMomentOnOpen(today, src_today);
}

TradeRecord System::runMomentOnClose(const Datetime& datetime) {
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _runMomentOnClose(today, src_today);
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

    m_buy_days++;
    m_sell_short_days++;
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
        // If a long position is held, liquidate and sell immediately
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_ENVIRONMENT);
            HKU_INFO_IF(trace, htr("[{}] EV to sell", name()));
        }

        m_pre_ev_valid = current_ev_valid;
        return tr.isNull() ? result : tr;
    }

    // When the environment changes from invalid to valid
    if (!m_pre_ev_valid) {
        HKU_INFO_IF(trace, htr("[{}] EV status from invalid to valid", name()));

        // If the environment strategy is used for the initial position building
        if (getParam<bool>("ev_open_position")) {
            HKU_INFO_IF(trace, htr("[{}] EV to buy", name()));
            TradeRecord tr = _buy(today, src_today, PART_ENVIRONMENT);
            m_pre_ev_valid = current_ev_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_ev_valid = current_ev_valid;

    //----------------------------------------------------------
    // Process the system valid condition strategy
    //----------------------------------------------------------

    bool current_cn_valid = _conditionIsValid(today.datetime);

    // If the system is currently invalid
    if (!current_cn_valid) {
        HKU_INFO_IF(trace, htr("[{}] current CN is invalid", name()));
        TradeRecord tr;
        // If a long position is held, liquidate and sell immediately
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_CONDITION);
            HKU_INFO_IF(trace, htr("[{}] CN to sell", name()));
        }

        m_pre_cn_valid = current_cn_valid;
        return tr.isNull() ? result : tr;
    }

    // If the system changes from invalid to valid
    if (!m_pre_cn_valid) {
        HKU_INFO_IF(trace, htr("[{}] CN status from invalid to valid", name()));

        // If the environment strategy is used for the initial position building
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
    if (m_sg->shouldSell(today.datetime)) {
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
    // The stop-loss uses the stop-loss price of the current position and the original price without
    // adjustment The profit goal uses the original price without adjustment
    //----------------------------------------------------------

    price_t current_price = today.closePrice;
    price_t src_current_price = src_today.closePrice;  // The original price without adjustment

    PositionRecord position = m_tm->getPosition(today.datetime, m_stock);
    HKU_INFO_IF(trace, htr("[{}] current position: {}", name(), position.number));
    if (position.number != 0) {
        TradeRecord tr;
        if (src_current_price <= position.stoploss) {
            HKU_INFO_IF(trace, htr("[{}] ST to sell, current price: {}, stoploss: {}", name(),
                                   src_current_price, position.stoploss));
            tr = _sell(today, src_today, PART_STOPLOSS);

        } else if (src_current_price >= _getGoalPrice(today.datetime, src_current_price)) {
            HKU_INFO_IF(trace, "[{}] {}: {}, {}: {}", name(), htr("PG to sell, current price"),
                        src_current_price, htr("goal price"),
                        _getGoalPrice(today.datetime, src_current_price));
            tr = _sell(today, src_today, PART_PROFITGOAL);

        } else {
            price_t current_take_profile = _getTakeProfitPrice(today.datetime, current_price);
            if (current_take_profile != 0.0) {
                if (current_take_profile < m_lastTakeProfit) {
                    current_take_profile = m_lastTakeProfit;
                } else {
                    m_lastTakeProfit = current_take_profile;
                }

                int tp_delay_n = getParam<int>("tp_delay_n");
                size_t pos = m_kdata.getPos(today.datetime);
                size_t position_pos = m_kdata.getPos(position.takeDatetime);
                // Sell when the current price is not higher than the take-profit price and the
                // take-profit delay condition is met
                price_t profit = position.number * src_today.closePrice - position.totalCost;
                if (pos - position_pos >= tp_delay_n && current_price <= current_take_profile &&
                    profit > (position.buyMoney - position.sellMoney)) {
                    HKU_INFO_IF(
                      trace,
                      htr("[{}] TP to sell, current price after restoration: {}, take_profit: {}",
                          name(), current_price, current_take_profile));
                    tr = _sell(today, src_today, PART_TAKEPROFIT);
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
    if (iszero(today.transAmount) || iszero(today.transCount)) {
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

    // Calculate the stop-loss price
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // Give up the trade when the planned price is not higher than the stop-loss price
    bool trace = getParam<bool>("trace");
    if (planPrice <= stoploss) {
        HKU_INFO_IF(trace, htr("[{}] buy failed, planPrice: {} <= stoploss: {}", name(), planPrice,
                               stoploss));
        return result;
    }

    // Get the buyable quantity
    double number = _getBuyNumber(today.datetime, planPrice, planPrice - stoploss, from);
    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    number = int64_t(number / min_num) * min_num;
    if (iszero(number) || number > m_stock.maxTradeNumber()) {
        HKU_INFO_IF(trace, "[{}] {}, number: {} == 0 or > maxTradeNumber: {}, {}", name(),
                    htr("buy failed"), number, m_stock.maxTradeNumber(), m_mm);
        return result;
    }

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    TradeRecord record =
      m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_BUY != record.business) {
        HKU_INFO_IF(trace, htr("[{}] buy failed, {}", name(), record));
        return result;
    }

    m_lastTakeProfit = record.realPrice;
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    return record;
}

TradeRecord System::_buyDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");

    // Delay the trade when the volume and the turnover amount are 0
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to buy, current amount == 0 or count == 0", name()));
        _submitBuyRequest(today, src_today, m_buyRequest.from);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's close price and check whether it is a one-line limit up
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", name()));
            _submitBuyRequest(today, src_today, m_buyRequest.from);
            return result;
        }

        const auto& pre_day = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > pre_day.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price up-limit board", name()));
            _submitBuyRequest(today, src_today, m_buyRequest.from);
            return result;
        }
    }

    // A delayed operation, take the open price of the current moment
    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // Calculate the stop-loss price and the buyable quantity
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // Calculate the stop-loss price and the buyable quantity with the current planned price
        stoploss = _getStoplossPrice(today, src_today, today.openPrice);
        number = planPrice <= stoploss ? 0.0
                                       : _getBuyNumber(today.datetime, planPrice,
                                                       planPrice - stoploss, m_buyRequest.from);
        goalPrice = _getGoalPrice(today.datetime, planPrice);

    } else {
        stoploss = m_buyRequest.stoploss;
        number = m_buyRequest.number;
        goalPrice = m_buyRequest.goal;
    }

    // If the planned buy price is not higher than the stop-loss price or the buy quantity is 0
    if (planPrice <= stoploss || number <= 0) {
        m_buyRequest.clear();
        return result;
    }

    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    number = int64_t(number / min_num) * min_num;

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                   planPrice, m_buyRequest.from);
    if (BUSINESS_BUY != record.business) {
        m_buyRequest.clear();
        return result;
    }

    m_buy_days = 0;
    m_lastTakeProfit = record.realPrice;
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyRequest.clear();
    return record;
}

void System::_submitBuyRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_buyRequest.valid) {
        if (m_buyRequest.count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_buyRequest.clear();
            return;
        }
        m_buyRequest.count++;

    } else {
        m_buyRequest.valid = true;
        m_buyRequest.business = BUSINESS_BUY;
        m_buyRequest.from = from;
        m_buyRequest.count = 1;
    }

    m_buyRequest.datetime = today.datetime;
    m_buyRequest.stoploss = _getStoplossPrice(today, src_today, today.closePrice);
    m_buyRequest.goal = _getGoalPrice(today.datetime, src_today.closePrice);
    m_buyRequest.number =
      _getBuyNumber(today.datetime, src_today.closePrice,
                    src_today.closePrice - m_buyRequest.stoploss, m_buyRequest.from);
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
    HKU_IF_RETURN(position.number <= 0.0, record);

    const auto& krecord = m_kdata.getKRecord(pos);
    const auto& src_krecord =
      m_stock.getKRecord(m_kdata.startPos() + pos, m_kdata.getQuery().kType());

    price_t realPrice =
      _getRealSellPrice(krecord.datetime, on_open ? src_krecord.openPrice : src_krecord.closePrice);

    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    // Round the quantity to be sold to an integer multiple of the minimum trade unit; when the
    // remainder is less than the minimum trade unit, sell everything at once
    double real_sell_num = static_cast<int64_t>(num / min_num) * min_num;
    if (position.number - real_sell_num < min_num) {
        real_sell_num = position.number;
    }

    record =
      m_tm->sell(date, m_stock, realPrice, real_sell_num, position.stoploss, position.goalPrice,
                 on_open ? src_krecord.openPrice : src_krecord.closePrice, from);
    HKU_WARN_IF_RETURN(record == Null<TradeRecord>(), record, "[{}] {}: {} by {}", name(),
                       htr("Failed force sell"), num, getSystemPartName(from));

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    return record;
}

TradeRecord System::_sell(const KRecord& today, const KRecord& src_today, Part from) {
    bool trace = getParam<bool>("trace");
    TradeRecord result;
    if (getParam<bool>("sell_delay")) {
        _submitSellRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will be delay to sell", name()));
        return result;
    }

    // Check whether it may be a one-line limit down
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] sell one-price board", name()));
            return _sellNow(today, src_today, from);
        }

        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name()));
            _submitSellRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name()));
            _submitSellRequest(today, src_today, from);
            return result;
        }
    }

    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", name()));
        _submitSellRequest(today, src_today, from);
        return result;
    }

    result = _sellNow(today, src_today, from);
    HKU_INFO_IF(trace, htr("[{}] sell now: {}", name(), result));
    return result;
}

TradeRecord System::_sellNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;
    double number = 0;

    // Calculate the new stop-loss price
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // When the new planned price is not higher than the new stop-loss price, the whole position is
    // to be sold
    number = _getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
    if (number <= 0) {
        return result;
    }

    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
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
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", name()));
        _submitSellRequest(today, src_today, m_sellRequest.from);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name()));
            _submitSellRequest(today, src_today, m_sellRequest.from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name()));
            _submitSellRequest(today, src_today, m_sellRequest.from);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // The stop-loss price at the moment the sell request is issued
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;

    Part from = m_sellRequest.from;

    if (getParam<bool>("delay_use_current_price")) {
        stoploss = _getStoplossPrice(today, src_today, today.openPrice);
        number = _getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
        goalPrice = _getGoalPrice(today.datetime, planPrice);
    } else {
        stoploss = m_sellRequest.stoploss;
        number = m_sellRequest.number;
        goalPrice = m_sellRequest.goal;
    }

    if (number <= 0) {
        m_sellRequest.clear();
        return result;
    }

    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sell(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                    planPrice, m_sellRequest.from);
    if (BUSINESS_SELL != record.business) {
        m_sellRequest.clear();
        return result;  // The sell operation failed
    }

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    } else {
        m_lastTakeProfit = src_today.openPrice;
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    m_sellRequest.clear();
    return record;
}

void System::_submitSellRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_sellRequest.valid) {
        if (m_sellRequest.count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_sellRequest.clear();
            return;
        }
        m_sellRequest.count++;

    } else {
        m_sellRequest.valid = true;
        m_sellRequest.business = BUSINESS_SELL;
        m_sellRequest.count = 1;
    }

    m_sellRequest.from = from;
    m_sellRequest.datetime = today.datetime;
    m_sellRequest.stoploss = _getStoplossPrice(today, src_today, today.closePrice);
    if (src_today.closePrice <= m_sellRequest.stoploss) {
        m_sellRequest.number = m_tm->getHoldNumber(today.datetime, m_stock);
    } else {
        m_sellRequest.number = _getSellNumber(today.datetime, src_today.closePrice,
                                              src_today.closePrice - m_sellRequest.stoploss, from);
    }

    m_sellRequest.goal = _getGoalPrice(today.datetime, src_today.closePrice);
}

TradeRecord System::_buyShort(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    if (getParam<bool>("support_borrow_stock") == false)
        return result;

    bool trace = getParam<bool>("trace");
    if (getParam<bool>("buy_delay")) {
        _submitBuyShortRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will buy short next bar open", name()));
        return result;
    }

    // The one-line board case
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy short one-price board", name()));
            return _buyShortNow(today, src_today, from);
        }

        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", name()));
            _submitBuyShortRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", name()));
            _submitBuyShortRequest(today, src_today, from);
            return result;
        }
    }

    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", name()));
        _submitBuyShortRequest(today, src_today, from);
        return result;
    }

    return _buyShortNow(today, src_today, from);
}

TradeRecord System::_buyShortNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;  // Take the close price of the current moment

    // Take the stop-loss price corresponding to the close price of the current moment
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    // Determine the quantity
    double number = _getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number <= 0) {
        m_buyShortRequest.clear();
        return result;
    }

    // Get the holding status of the current short position
    PositionRecord pos = m_tm->getShortPosition(m_stock);
    if (pos.number <= 0) {
        m_buyShortRequest.clear();
        return result;
    }

    if (number > pos.number) {
        number = pos.number;
    }

    price_t goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);

    TradeRecord record = m_tm->buyShort(today.datetime, m_stock, realPrice, number, stoploss,
                                        goalPrice, planPrice, PART_SIGNAL);
    if (BUSINESS_BUY_SHORT != record.business) {
        m_buyShortRequest.clear();
        return result;
    }

    m_sell_short_days = 0;
    m_lastTakeProfit = realPrice;  // The take-profit is assigned the buy price
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return record;
}

TradeRecord System::_buyShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", name()));
        _submitBuyShortRequest(today, src_today, m_buyShortRequest.from);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", name()));
            _submitBuyShortRequest(today, src_today, m_buyShortRequest.from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", name()));
            _submitBuyShortRequest(today, src_today, m_buyShortRequest.from);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the close price of the current moment

    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // Take the stop-loss price corresponding to the close price of the current moment
        stoploss = _getShortStoplossPrice(today, src_today, today.openPrice);
        number = _getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice,
                                    m_buyShortRequest.from);
        goalPrice = _getShortGoalPrice(today.datetime, planPrice);

    } else {
        stoploss = m_buyShortRequest.stoploss;
        number = m_buyShortRequest.number;
        goalPrice = m_buyShortRequest.goal;
    }

    if (number <= 0) {
        m_buyShortRequest.clear();
        return result;
    }

    // Get the holding status of the current short position
    PositionRecord pos = m_tm->getShortPosition(m_stock);
    if (pos.number <= 0) {
        m_buyShortRequest.clear();
        return result;
    }

    if (number > pos.number) {
        number = pos.number;
    }

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->buyShort(today.datetime, m_stock, realPrice, number, stoploss,
                                        goalPrice, planPrice, PART_SIGNAL);
    if (BUSINESS_BUY_SHORT != record.business) {
        m_buyShortRequest.clear();
        return result;
    }

    m_sell_short_days = 0;
    m_lastTakeProfit = realPrice;  // The take-profit is assigned the buy price
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return result;
}

void System::_submitBuyShortRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_buyShortRequest.valid) {
        if (m_buyShortRequest.count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_buyShortRequest.clear();
            return;
        }
        m_buyShortRequest.count++;

    } else {
        m_buyShortRequest.valid = true;
        m_buyShortRequest.business = BUSINESS_BUY;
        m_buyShortRequest.from = from;
        m_buyShortRequest.count = 1;
    }

    m_buyShortRequest.datetime = today.datetime;
    m_buyShortRequest.stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);
    m_buyShortRequest.goal = _getShortGoalPrice(today.datetime, src_today.closePrice);
    m_buyShortRequest.number =
      _getBuyShortNumber(today.datetime, src_today.closePrice,
                         m_buyShortRequest.stoploss - src_today.closePrice, m_buyShortRequest.from);
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

    if (iszero(today.transAmount) || iszero(today.transCount)) {
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

    // Calculate the stop-loss price
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    double number = _getSellShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number <= 0) {
        m_sellShortRequest.clear();
        return result;
    }

    price_t goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                         goalPrice, planPrice, PART_SIGNAL);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  // The sell operation failed
    }

    m_sell_short_days = 0;
    m_lastShortTakeProfit = realPrice;
    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    m_sellShortRequest.clear();
    return record;
}

TradeRecord System::_sellShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to sell short, current amount == 0 or count == 0", name()));
        _submitSellShortRequest(today, src_today, m_sellShortRequest.from);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell short, one-price board", name()));
            _submitSellShortRequest(today, src_today, m_sellShortRequest.from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short selling delayed: limit-down lock", name()));
            _submitSellShortRequest(today, src_today, m_sellShortRequest.from);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // The stop-loss price at the moment the sell request is issued
    price_t stoploss = 0.0;
    double number = 0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        stoploss = _getShortStoplossPrice(today, src_today, today.openPrice);
        number = _getSellShortNumber(today.datetime, planPrice, stoploss - planPrice,
                                     m_sellShortRequest.from);
        goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    } else {
        stoploss = m_sellShortRequest.stoploss;
        number = m_sellShortRequest.number;
        goalPrice = m_sellShortRequest.goal;
    }

    if (number <= 0) {
        m_sellShortRequest.clear();
        return result;
    }

    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);

    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                         goalPrice, planPrice, m_sellShortRequest.from);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  // The sell operation failed
    }

    m_sell_short_days = 0;
    m_lastShortTakeProfit = realPrice;
    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    m_sellShortRequest.clear();
    return record;
}

void System::_submitSellShortRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_sellShortRequest.valid) {
        if (m_sellShortRequest.count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_sellShortRequest.clear();
            return;
        }
        m_sellShortRequest.count++;

    } else {
        m_sellShortRequest.valid = true;
        m_sellShortRequest.business = BUSINESS_SELL_SHORT;
        m_sellShortRequest.from = from;
        m_sellShortRequest.count = 1;
    }

    m_sellShortRequest.datetime = today.datetime;
    m_sellShortRequest.stoploss = _getStoplossPrice(today, src_today, today.closePrice);
    m_sellShortRequest.goal = _getGoalPrice(today.datetime, src_today.closePrice);
    m_sellShortRequest.number =
      _getSellNumber(today.datetime, src_today.closePrice,
                     src_today.closePrice - m_sellShortRequest.stoploss, m_sellShortRequest.from);
}

TradeRecord System::_processRequest(const KRecord& today, const KRecord& src_today) {
    HKU_IF_RETURN(m_buyRequest.valid, _buyDelay(today, src_today));
    HKU_IF_RETURN(m_sellRequest.valid, _sellDelay(today, src_today));
    HKU_IF_RETURN(m_sellShortRequest.valid, _sellShortDelay(today, src_today));
    HKU_IF_RETURN(m_buyShortRequest.valid, _buyShortDelay(today, src_today));
    return TradeRecord();
}

TradeRecord System::pfProcessDelaySellRequest(const Datetime& date) {
    HKU_IF_RETURN(!m_sellRequest.valid, TradeRecord());
    size_t pos = m_kdata.getPos(date);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _sellDelay(today, src_today);
}

TradeRecord System::pfProcessDelayBuyRequest(const Datetime& date) {
    HKU_IF_RETURN(!m_buyRequest.valid, TradeRecord());
    size_t pos = m_kdata.getPos(date);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _buyDelay(today, src_today);
}

price_t System::_getStoplossPrice(const KRecord& today, const KRecord& src_today, price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    HKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
    price_t stoploss = m_st->getPrice(today.datetime, price);
    price_t adjust = (stoploss - today.lowPrice) / (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                     src_today.lowPrice;
    return adjust >= 0.0 ? adjust : 0.0;
}

price_t System ::_getShortStoplossPrice(const KRecord& today, const KRecord& src_today,
                                        price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    HKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
    price_t stoploss = m_st->getShortPrice(today.datetime, price);
    price_t adjust = (stoploss - today.lowPrice) / (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                     src_today.lowPrice;
    return adjust >= 0.0 ? adjust : 0.0;
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
        auto tr_list = m_tm->getTradeList();
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
    if (m_buyRequest.valid) {
        json buy_request;
        buy_request["stock"] = m_stock.market_code();
        buy_request["business"] = getBusinessName(m_buyRequest.business);
        buy_request["datetime"] = m_buyRequest.datetime.str();
        buy_request["stoploss"] = m_buyRequest.stoploss;
        buy_request["goal"] = m_buyRequest.goal;
        buy_request["number"] = m_buyRequest.number;
        buy_request["from"] = getSystemPartName(m_buyRequest.from);
        buy_request["remark"] = m_buyRequest.remark;
        buy_request["count"] = m_buyRequest.count;
        delay_on_next_open.push_back(buy_request);
    }

    if (m_sellRequest.valid) {
        json sell_request;
        sell_request["stock"] = m_stock.market_code();
        sell_request["business"] = getBusinessName(m_sellRequest.business);
        sell_request["datetime"] = m_sellRequest.datetime.str();
        sell_request["stoploss"] = m_sellRequest.stoploss;
        sell_request["goal"] = m_sellRequest.goal;
        sell_request["number"] = m_sellRequest.number;
        sell_request["from"] = getSystemPartName(m_sellRequest.from);
        sell_request["remark"] = m_sellRequest.remark;
        sell_request["count"] = m_sellRequest.count;
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
