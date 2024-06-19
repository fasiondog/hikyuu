/*
 * SystemBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "hikyuu/global/sysinfo.h"
#include "System.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const System& sys) {
    string strip(",\n");
    string space("  ");
    os << "System{\n"
       << space << sys.name() << strip << space << sys.getTO().getQuery() << strip << space
       << sys.getStock() << strip << space << sys.getParameter() << strip << space << sys.getEV()
       << strip << space << sys.getCN() << strip << space << sys.getMM() << strip << space
       << sys.getSG() << strip << space << sys.getST() << strip << space << sys.getTP() << strip
       << space << sys.getPG() << strip << space << sys.getSP() << strip << space
       << (sys.getTM() ? sys.getTM()->str() : "TradeManager(NULL)") << strip << "}";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SystemPtr& sys) {
    if (sys) {
        os << *sys;
    } else {
        os << "System(NULL)";
    }
    return os;
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

    // 连续延迟交易请求的限制次数，需大于等于0，0表示只允许延迟1次
    setParam<int>("max_delay_count", 3);

    // 是否延迟到下一个bar开盘时进行交易
    setParam<bool>("buy_delay", true);  // 非延迟操作取当前Bar的收盘价操作；延迟取下一BAR开盘价
    setParam<bool>("sell_delay", true);

    // 延迟操作的情况下，是使用当前的价格计算新的止损价/止赢价/目标价还是使用上次计算的结果
    setParam<bool>("delay_use_current_price", true);
    setParam<bool>("tp_monotonic", true);  // 止赢单调递增
    setParam<int>("tp_delay_n", 1);        // 止赢延迟判断天数
    setParam<bool>("ignore_sell_sg", false);  // 忽略卖出信号，只使用止损/止赢等其他方式卖出

    // 最高价等于最低价时，是否可进行交易
    setParam<bool>("can_trade_when_high_eq_low", false);

    // 是否使用市场环境判定进行初始建仓
    setParam<bool>("ev_open_position", false);

    // 是否使用系统有效性条件进行初始建仓
    setParam<bool>("cn_open_position", false);

    // 在现金不足时，是否支持借入现金，融资
    setParam<bool>("support_borrow_cash", false);

    // 在没有持仓时，是否支持借入证券，融券
    setParam<bool>("support_borrow_stock", false);

    // 以下参数控制各个部件的共享策略，影响 clone 和 reset 操作
    // 当为共享组件时，不会 clone 和 reset 相应的组件
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

void System::baseCheckParam(const string& name) const {
    if ("max_delay_count" == name) {
        HKU_ASSERT(getParam<int>("max_delay_count") >= 0);
    } else if ("tp_delay_n" == name) {
        HKU_ASSERT(getParam<int>("tp_delay_n") >= 0);
    } else if ("trace" == name) {
        if (getParam<bool>("trace") && pythonInJupyter()) {
            HKU_THROW("You can't trace in jupyter!");
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

    // 不能复位m_stock / m_kdata/
    //  m_src_kdata，后续Portfolio需要使用，从意义上讲，sys实例和stock是一一绑定的关系,
    // 一个sys实例绑定stock后，除非主动改变，否则不应该被reset
    //  m_stock

    m_calculated = false;
    m_pre_ev_valid = false;  // true;
    m_pre_cn_valid = false;  // true;

    m_buy_days = 0;
    m_sell_short_days = 0;
    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
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

    // 清理交易对象
    m_stock = Null<Stock>();
    m_src_kdata = Null<KData>();
    m_kdata = Null<KData>();

    m_calculated = false;
    m_pre_ev_valid = false;  // true;
    m_pre_cn_valid = false;  // true;

    m_buy_days = 0;
    m_sell_short_days = 0;
    m_trade_list.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;

    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

void System::setTO(const KData& kdata) {
    if (m_kdata != kdata) {
        m_calculated = false;
        m_kdata = kdata;
    }

    HKU_TRACE_IF_RETURN(m_calculated, void(), "No need to calcule!");

    m_stock = kdata.getStock();
    KQuery query = kdata.getQuery();
    if (m_stock.isNull() || query.recoverType() == KQuery::NO_RECOVER) {
        m_src_kdata = m_kdata;
    } else {
        KQuery no_recover_query = query;
        no_recover_query.recoverType(KQuery::NO_RECOVER);
        m_src_kdata = m_stock.getKData(no_recover_query);
    }

    HKU_WARN_IF(
      query.recoverType() == KQuery::FORWARD || query.recoverType() == KQuery::EQUAL_FORWARD,
      "You are using forward or equal_forward kdata, which is a future function!");

    // sg->setTO必须在cn->setTO之前，cn会使用到sg，防止sg被计算两次
    if (m_sg)
        m_sg->setTO(kdata);  // 传入复权的 KData
    if (m_cn)
        m_cn->setTO(kdata);  // 传入复权的 KData
    if (m_st)
        m_st->setTO(kdata);  // 传入复权的 KData
    if (m_tp)
        m_tp->setTO(kdata);  // 传入复权的 KData
    if (m_pg)
        m_pg->setTO(m_src_kdata);  // 传入原始未复权的 KData
    if (m_sp)
        m_sp->setTO(m_src_kdata);  // 传入原始未复权的 KData

    if (m_ev)
        m_ev->setQuery(query);
    if (m_mm)
        m_mm->setQuery(query);
}

SystemPtr System::clone() {
    SystemPtr p = make_shared<System>();
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

    // 如果存在市场环境判断策略，则需要将默认的前一日市场有效标志置为false
    // 因为需要由市场环境判断策略全权判定市场是否有效
    if (m_ev)
        m_pre_ev_valid = false;

    if (m_cn) {
        m_cn->setTM(m_tm);
        m_cn->setSG(m_sg);
        m_pre_cn_valid = false;  // 默认的前一日市场有效标志置为false
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
    // reset必须在readyForRun之前，否则m_pre_cn_valid、m_pre_ev_valid将会被赋为错误的初值
    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    HKU_DEBUG_IF_RETURN(m_calculated && m_kdata == kdata, void(), "Not need calculate.");

    readyForRun();

    bool trace = getParam<bool>("trace");
    setTO(kdata);
    size_t total = kdata.size();
    auto const* ks = kdata.data();
    auto const* src_ks = m_src_kdata.data();
    for (size_t i = 0; i < total; ++i) {
        if (ks[i].datetime >= m_tm->initDatetime()) {
            auto tr = _runMoment(ks[i], src_ks[i]);
            if (trace) {
                HKU_INFO_IF(!tr.isNull(), "{}", tr);
                PositionRecord position = m_tm->getPosition(ks[i].datetime, m_stock);
                FundsRecord funds = m_tm->getFunds(kdata.getQuery().kType());
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

bool System::haveDelaySellRequest() const {
    return m_sellRequest.valid;
}

TradeRecord System::runMoment(const Datetime& datetime) {
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _runMoment(today, src_today);
}

TradeRecord System::_runMoment(const KRecord& today, const KRecord& src_today) {
    bool trace = getParam<bool>("trace");
    if (trace) {
        HKU_INFO("{} ------------------------------------------------------", today.datetime);
        HKU_INFO("[{}] source {} ", name(), src_today);
        HKU_INFO_IF(m_kdata.getQuery().recoverType() != KQuery::NO_RECOVER, "[{}] Restitution {} ",
                    name(), src_today);
    }

    m_buy_days++;
    m_sell_short_days++;
    TradeRecord result;
    if ((today.highPrice == today.lowPrice || today.closePrice > today.highPrice ||
         today.closePrice < today.lowPrice) &&
        !getParam<bool>("can_trade_when_high_eq_low")) {
        HKU_INFO_IF(trace, "[{}] ignore current highPrice == lowPrice", name());
        return result;
    }

    // 处理当前已有的交易请求
    result = _processRequest(today, src_today);

    //----------------------------------------------------------
    // 处理市场环境策略
    //----------------------------------------------------------

    bool current_ev_valid = _environmentIsValid(today.datetime);

    // 如果当前环境无效
    if (!current_ev_valid) {
        HKU_INFO_IF(trace, "[{}] current EV is invalid", name());
        TradeRecord tr;
        // 如果持有多头仓位，则立即清仓卖出
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_ENVIRONMENT);
            HKU_INFO_IF(trace, "[{}] EV to sell", name());
        }

        m_pre_ev_valid = current_ev_valid;
        return tr.isNull() ? result : tr;
    }

    // 环境是从无效变为有效时
    if (!m_pre_ev_valid) {
        HKU_INFO_IF(trace, "[{}] EV status from invalid to valid", name());

        // 如果使用环境判定策略进行初始建仓
        if (getParam<bool>("ev_open_position")) {
            HKU_INFO_IF(trace, "[{}] EV to buy", name());
            TradeRecord tr = _buy(today, src_today, PART_ENVIRONMENT);
            m_pre_ev_valid = current_ev_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_ev_valid = current_ev_valid;

    //----------------------------------------------------------
    // 处理系统有效条件判断策略
    //----------------------------------------------------------

    bool current_cn_valid = _conditionIsValid(today.datetime);

    // 如果系统当前无效
    if (!current_cn_valid) {
        HKU_INFO_IF(trace, "[{}] current CN is invalid", name());
        TradeRecord tr;
        // 如果持有多头仓位，则立即清仓卖出
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_CONDITION);
            HKU_INFO_IF(trace, "[{}] CN to sell", name());
        }

        m_pre_cn_valid = current_cn_valid;
        return tr.isNull() ? result : tr;
    }

    // 如果系统从无效变为有效
    if (!m_pre_cn_valid) {
        HKU_INFO_IF(trace, "[{}] CN status from invalid to valid", name());

        // 如果使用环境判定策略进行初始建仓
        if (getParam<bool>("cn_open_position")) {
            HKU_INFO_IF(trace, "[{}] CN to buy", name());
            TradeRecord tr = _buy(today, src_today, PART_CONDITION);
            m_pre_cn_valid = current_cn_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_cn_valid = current_cn_valid;

    //----------------------------------------------------------
    // 处理买入、卖出信号
    //----------------------------------------------------------

    // 如果有买入信号
    if (m_sg->shouldBuy(today.datetime)) {
        TradeRecord tr;
        if (m_tm->haveShort(m_stock)) {
            HKU_INFO_IF(trace, "[{}] SG to buy short", name());
            tr = _buyShort(today, src_today, PART_SIGNAL);

        } else {
            HKU_INFO_IF(trace, "[{}] SG to buy", name());
            tr = _buy(today, src_today, PART_SIGNAL);
        }
        return tr.isNull() ? result : tr;
    }

    // 发出卖出信号
    if (m_sg->shouldSell(today.datetime)) {
        TradeRecord tr;
        if (m_tm->have(m_stock)) {
            HKU_INFO_IF(trace, "[{}] SG to sell", name());
            tr = _sell(today, src_today, PART_SIGNAL);

        } else {
            HKU_INFO_IF(trace, "[{}] SG to sell short", name());
            tr = _sellShort(today, src_today, PART_SIGNAL);
        }
        return tr.isNull() ? result : tr;
    }

    //----------------------------------------------------------
    // 处理止损、止盈、目标信号
    // 止损使用的是当前持仓的止损价，使用未复权的原始价
    // 目标盈利使用未复权的原始价格
    //----------------------------------------------------------

    price_t current_price = today.closePrice;
    price_t src_current_price = src_today.closePrice;  // 未复权的原始价格

    PositionRecord position = m_tm->getPosition(today.datetime, m_stock);
    HKU_INFO_IF(trace, "[{}] current postion: {}", name(), position.number);
    if (position.number != 0) {
        TradeRecord tr;
        if (src_current_price <= position.stoploss) {
            HKU_INFO_IF(trace, "[{}] ST to sell, current price: {}, stoploss: {}", name(),
                        src_current_price, position.stoploss);
            tr = _sell(today, src_today, PART_STOPLOSS);

        } else if (src_current_price >= _getGoalPrice(today.datetime, src_current_price)) {
            HKU_INFO_IF(trace, "[{}] PG to sell, current price: {}, goal: {} ", name(),
                        src_current_price, _getGoalPrice(today.datetime, src_current_price));
            tr = _sell(today, src_today, PART_PROFITGOAL);

        } else {
            price_t current_take_profile =
              _getTakeProfitPrice(today.datetime, src_today.closePrice);
            if (current_take_profile != 0.0) {
                if (current_take_profile < m_lastTakeProfit) {
                    current_take_profile = m_lastTakeProfit;
                } else {
                    m_lastTakeProfit = current_take_profile;
                }

                int tp_delay_n = getParam<int>("tp_delay_n");
                size_t pos = m_kdata.getPos(today.datetime);
                size_t position_pos = m_kdata.getPos(position.takeDatetime);
                // 如果当前价格小于等于止盈价，且满足止盈延迟条件则卖出
                price_t profit = position.number * src_today.closePrice - position.totalCost;
                if (pos - position_pos >= tp_delay_n && current_price <= current_take_profile &&
                    profit > (position.buyMoney - position.sellMoney)) {
                    HKU_INFO_IF(
                      trace,
                      "[{}] TP to sell, current price after restoration: {}, take_profit: {}",
                      name(), current_price, current_take_profile);
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
    if (getParam<bool>("buy_delay")) {
        _submitBuyRequest(today, src_today, from);
        return result;
    } else {
        return _buyNow(today, src_today, from);
    }
}

TradeRecord System::_buyNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;

    // 以当前收盘价为计划价格
    price_t planPrice = src_today.closePrice;

    // 计算止损价
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // 如果计划的价格已经小于等于止损价，放弃交易
    if (planPrice <= stoploss) {
        return result;
    }

    // 获取可买入数量
    double number = _getBuyNumber(today.datetime, planPrice, planPrice - stoploss, from);
    double min_num = m_stock.minTradeNumber();
    number = int64_t(number / min_num) * min_num;
    if (number == 0 || number > m_stock.maxTradeNumber()) {
        return result;
    }

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    TradeRecord record =
      m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_BUY != record.business) {
        return result;
    }

    m_lastTakeProfit = record.realPrice;
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    return record;
}

TradeRecord System::_buyDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // 无法实际执行，延迟至下一时刻
        _submitBuyRequest(KRecord(today.datetime), KRecord(today.datetime), m_buyRequest.from);
        return result;
    }

    // 延迟操作，取当前时刻开盘价
    price_t planPrice = src_today.openPrice;  // 取当前时刻的开盘价

    // 计算止损价和可买入数量
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // 使用当前计划价格计算止损价和可买入数量
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

    // 如果计划买入的价格已经小于等于止损价或者买入数量等于0
    if (planPrice <= stoploss || number == 0) {
        m_buyRequest.clear();
        return result;
    }

    double min_num = m_stock.minTradeNumber();
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
            // 超出最大延迟次数，清除买入请求
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
    HKU_INFO_IF(trace, "[{}] force sell {} by {}", name(), num, getSystemPartName(from));

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
    // 对待卖出的数量进行最小交易单位取整数倍，如果剩余不足最小交易单位，则一次全部卖出
    double real_sell_num = static_cast<int64_t>(num / min_num) * min_num;
    if (position.number - real_sell_num < min_num) {
        real_sell_num = position.number;
    }

    record =
      m_tm->sell(date, m_stock, realPrice, real_sell_num, position.stoploss, position.goalPrice,
                 on_open ? src_krecord.openPrice : src_krecord.closePrice, from);

    // 如果已未持仓，最后的止赢价初始为0
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
        HKU_INFO_IF(trace, "[{}] will be delay to sell", name());
        return result;
    } else {
        result = _sellNow(today, src_today, from);
        HKU_INFO_IF(trace, "[{}] sell now: {}", name(), result);
        return result;
    }
}

TradeRecord System::_sellNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;
    double number = 0;

    // 计算新的止损价
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    // 如果新的计划价格已经小于等于新的止损价，则认为需全部卖出
    if (planPrice <= stoploss) {
        number = m_tm->getHoldNumber(today.datetime, m_stock);

        // 否则，认为可能只是进行减仓操作
    } else {
        number = _getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
        if (number == 0) {
            return result;
        }
    }

    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record =
      m_tm->sell(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_SELL != record.business) {
        return result;  // 卖出操作失败
    }

    // 如果已未持仓，最后的止赢价初始为0
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
    HKU_INFO_IF(trace, "[{}] process _sellDelay request", name());

    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // 无法执行，保留卖出请求，继续延迟至下一时刻
        _submitSellRequest(KRecord(today.datetime), KRecord(today.datetime), m_sellRequest.from);
        return result;
    }

    price_t planPrice = src_today.openPrice;  // 取当前时刻的开盘价

    // 发出卖出请求时刻的止损价
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;

    Part from = m_sellRequest.from;

    if (getParam<bool>("delay_use_current_price")) {
        stoploss = _getStoplossPrice(today, src_today, today.openPrice);
        if (planPrice < stoploss) {
            number = m_tm->getHoldNumber(today.datetime, m_stock);
        } else {
            number = _getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
        }
        goalPrice = _getGoalPrice(today.datetime, planPrice);
    } else {
        stoploss = m_sellRequest.stoploss;
        number = m_sellRequest.number;
        goalPrice = m_sellRequest.goal;
    }

    if (number == 0) {
        m_sellRequest.clear();
        return result;
    }

    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sell(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                    planPrice, m_sellRequest.from);
    if (BUSINESS_SELL != record.business) {
        m_sellRequest.clear();
        return result;  // 卖出操作失败
    }

    // 如果已未持仓，最后的止赢价初始为0
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
            // 超出最大延迟次数，清除买入请求
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

    if (getParam<bool>("buy_delay")) {
        _submitBuyShortRequest(today, src_today, from);
        return result;
    } else {
        return _buyShortNow(today, src_today, from);
    }
}

TradeRecord System::_buyShortNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice) {
        // 无法实际执行，延迟至下一时刻
        //_submitBuyRequest(m_buyRequest.datetime);
        return result;
    }

    price_t planPrice = src_today.closePrice;  // 取当前时刻的收盘价

    // 取当前时刻的收盘价对应的止损价
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    // 确定数量
    double number = _getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number == 0) {
        m_buyShortRequest.clear();
        return result;
    }

    // 获取当前空头仓位持有情况
    PositionRecord pos = m_tm->getShortPosition(m_stock);
    if (pos.number == 0) {
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
    m_lastTakeProfit = realPrice;  // 止赢赋值给买入价格
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return record;
}

TradeRecord System::_buyShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice) {
        // 无法实际执行，延迟至下一时刻
        //_submitBuyRequest(m_buyRequest.datetime);
        return result;
    }

    price_t planPrice = src_today.openPrice;  // 取当前时刻的收盘价

    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // 取当前时刻的收盘价对应的止损价
        stoploss = _getShortStoplossPrice(today, src_today, today.openPrice);
        number =
          _getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, m_buyRequest.from);
        goalPrice = _getShortGoalPrice(today.datetime, planPrice);

    } else {
        stoploss = m_buyShortRequest.stoploss;
        number = m_buyShortRequest.number;
        goalPrice = m_buyShortRequest.goal;
    }

    if (number == 0) {
        m_buyShortRequest.clear();
        return result;
    }

    // 获取当前空头仓位持有情况
    PositionRecord pos = m_tm->getShortPosition(m_stock);
    if (pos.number == 0) {
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
    m_lastTakeProfit = realPrice;  // 止赢赋值给买入价格
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return result;
}

void System::_submitBuyShortRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_buyShortRequest.valid) {
        if (m_buyShortRequest.count > getParam<int>("max_delay_count")) {
            // 超出最大延迟次数，清除买入请求
            m_buyRequest.clear();
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
    if (getParam<bool>("support_borrow_stock") == false) {
        // HKU_WARN("set system param support_borrow_stock to true to short sell");
        return result;
    }

    if (getParam<bool>("sell_delay")) {
        _submitSellShortRequest(today, src_today, from);
        return result;
    } else {
        return _sellShortNow(today, src_today, from);
    }
}

TradeRecord System::_sellShortNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice) {
        // 当前无法卖出，延迟至下一时刻卖出
        _submitSellShortRequest(today, src_today, from);
        return result;
    }

    price_t planPrice = src_today.closePrice;

    // 计算止损价
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    double number = _getSellShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number == 0) {
        m_sellShortRequest.clear();
        return result;
    }

    price_t goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                         goalPrice, planPrice, PART_SIGNAL);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  // 卖出操作失败
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
    if (today.highPrice == today.lowPrice) {
        // 无法执行，保留卖出请求，继续延迟至下一时刻
        _submitSellShortRequest(today, src_today, m_sellShortRequest.from);
        return result;
    }

    price_t planPrice = src_today.openPrice;  // 取当前时刻的开盘价

    // 发出卖出请求时刻的止损价
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

    if (number == 0) {
        m_sellShortRequest.clear();
        return result;
    }

    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);

    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss,
                                         goalPrice, planPrice, m_sellShortRequest.from);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  // 卖出操作失败
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
            // 超出最大延迟次数，清除买入请求
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

} /* namespace hku */
