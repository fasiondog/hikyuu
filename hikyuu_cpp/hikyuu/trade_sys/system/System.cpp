/*
 * SystemBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

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
    //连续延迟交易请求的限制次数，需大于等于0，0表示只允许延迟1次
    setParam<int>("max_delay_count", 3);

    //是否延迟到下一个bar开盘时进行交易
    setParam<bool>("buy_delay", true);  //非延迟操作取当前Bar的收盘价操作；延迟取下一BAR开盘价
    setParam<bool>("sell_delay", true);

    //延迟操作的情况下，是使用当前的价格计算新的止损价/止赢价/目标价还是使用上次计算的结果
    setParam<bool>("delay_use_current_price", true);
    setParam<bool>("tp_monotonic", true);  //止赢单调递增
    setParam<int>("tp_delay_n", 3);        //止赢延迟判断天数
    setParam<bool>("ignore_sell_sg", false);  //忽略卖出信号，只使用止损/止赢等其他方式卖出

    //最高价等于最低价时，是否可进行交易
    setParam<bool>("can_trade_when_high_eq_low", false);

    //是否使用市场环境判定进行初始建仓
    setParam<bool>("ev_open_position", false);

    //是否使用系统有效性条件进行初始建仓
    setParam<bool>("cn_open_position", false);

    //在现金不足时，是否支持借入现金，融资
    setParam<bool>("support_borrow_cash", false);

    //在没有持仓时，是否支持借入证券，融券
    setParam<bool>("support_borrow_stock", false);
}

void System::reset(bool with_tm, bool with_ev) {
    if (with_tm && m_tm)
        m_tm->reset();
    if (with_ev && m_ev)
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

    //不能复位m_stock / m_kdata/
    // m_src_kdata，后续Portfolio需要使用，从意义上讲，sys实例和stock是一一绑定的关系,
    //一个sys实例绑定stock后，除非主动改变，否则不应该被reset
    // m_stock

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
    m_kdata = kdata;
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
        p->m_tm = m_tm->clone();
    // ev 通常作为公共组件不进行克隆
    // if (m_ev)
    //     p->m_ev = m_ev->clone();
    if (m_mm)
        p->m_mm = m_mm->clone();
    if (m_cn)
        p->m_cn = m_cn->clone();
    if (m_sg)
        p->m_sg = m_sg->clone();
    if (m_st)
        p->m_st = m_st->clone();
    if (m_tp)
        p->m_tp = m_tp->clone();
    if (m_pg)
        p->m_pg = m_pg->clone();
    if (m_sp)
        p->m_sp = m_sp->clone();

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_stock = m_stock;
    p->m_kdata = m_kdata;
    p->m_src_kdata = m_src_kdata;

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

bool System::readyForRun() {
    HKU_ERROR_IF_RETURN(!m_tm, false, "Not setTradeManager!");
    HKU_ERROR_IF_RETURN(!m_mm, false, "Not setMoneyManager!");
    HKU_ERROR_IF_RETURN(!m_sg, false, "Not setSignal!");

    //如果存在市场环境判断策略，则需要将默认的前一日市场有效标志置为false
    //因为需要由市场环境判断策略全权判定市场是否有效
    if (m_ev)
        m_pre_ev_valid = false;

    if (m_cn) {
        m_cn->setTM(m_tm);
        m_cn->setSG(m_sg);
        m_pre_cn_valid = false;  //默认的前一日市场有效标志置为false
    }

    if (m_mm)
        m_mm->setTM(m_tm);
    if (m_pg)
        m_pg->setTM(m_tm);
    if (m_st)
        m_st->setTM(m_tm);
    if (m_tp)
        m_tp->setTM(m_tm);

    m_tm->setParam<bool>("support_borrow_cash", getParam<bool>("support_borrow_cash"));
    m_tm->setParam<bool>("support_borrow_stock", getParam<bool>("support_borrow_stock"));

    return true;
}

void System::run(const KQuery& query, bool reset) {
    HKU_ERROR_IF_RETURN(m_stock.isNull(), void(), "m_stock is NULL!");

    // reset必须在readyForRun之前，否则m_pre_cn_valid、m_pre_ev_valid将会被赋为错误的初值
    if (reset)
        this->reset(true, true);

    HKU_IF_RETURN(!readyForRun(), void());

    // m_stock = stock; 在setTO里赋值
    KData kdata = m_stock.getKData(query);
    HKU_IF_RETURN(kdata.empty(), void());

    setTO(kdata);
    size_t total = kdata.size();
    for (size_t i = 0; i < total; ++i) {
        if (kdata[i].datetime >= m_tm->initDatetime()) {
            _runMoment(kdata[i], m_src_kdata[i]);
        }
    }
}

void System::run(const Stock& stock, const KQuery& query, bool reset) {
    m_stock = stock;
    run(query, reset);
}

void System::run(const KData& kdata, bool reset) {
    HKU_INFO_IF_RETURN(kdata.empty(), void(), "Input kdata is empty!");

    // reset必须在readyForRun之前，否则m_pre_cn_valid、m_pre_ev_valid将会被赋为错误的初值
    if (reset)
        this->reset(true, true);

    HKU_IF_RETURN(!readyForRun(), void());

    setTO(kdata);
    size_t total = kdata.size();
    for (size_t i = 0; i < total; ++i) {
        if (kdata[i].datetime >= m_tm->initDatetime()) {
            _runMoment(kdata[i], m_src_kdata[i]);
        }
    }
}

void System::clearDelayRequest() {
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

TradeRecord System::runMoment(const Datetime& datetime) {
    size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());

    KRecord today = m_kdata.getKRecord(pos);
    KRecord src_today = m_src_kdata.getKRecord(pos);
    return _runMoment(today, src_today);
}

TradeRecord System::_runMoment(const KRecord& today, const KRecord& src_today) {
    m_buy_days++;
    m_sell_short_days++;
    TradeRecord result;
    if ((today.highPrice == today.lowPrice || today.closePrice > today.highPrice ||
         today.closePrice < today.lowPrice) &&
        !getParam<bool>("can_trade_when_high_eq_low")) {
        return result;
    }

    //处理当前已有的交易请求
    result = _processRequest(today, src_today);

    //----------------------------------------------------------
    // 处理市场环境策略
    //----------------------------------------------------------

    bool current_ev_valid = _environmentIsValid(today.datetime);

    //如果当前环境无效
    if (!current_ev_valid) {
        TradeRecord tr;
        //如果持有多头仓位，则立即清仓卖出
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_ENVIRONMENT);
        }

        m_pre_ev_valid = current_ev_valid;
        return tr.isNull() ? result : tr;
    }

    //环境是从无效变为有效时
    if (!m_pre_ev_valid) {
        //如果使用环境判定策略进行初始建仓
        if (getParam<bool>("ev_open_position")) {
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

    //如果系统当前无效
    if (!current_cn_valid) {
        TradeRecord tr;
        //如果持有多头仓位，则立即清仓卖出
        if (m_tm->have(m_stock)) {
            tr = _sell(today, src_today, PART_CONDITION);
        }

        m_pre_cn_valid = current_cn_valid;
        return tr.isNull() ? result : tr;
    }

    //如果系统从无效变为有效
    if (!m_pre_cn_valid) {
        //如果使用环境判定策略进行初始建仓
        if (getParam<bool>("cn_open_position")) {
            TradeRecord tr = _buy(today, src_today, PART_CONDITION);
            m_pre_cn_valid = current_cn_valid;
            return tr.isNull() ? result : tr;
        }
    }

    m_pre_cn_valid = current_cn_valid;

    //----------------------------------------------------------
    // 处理买入、卖出信号
    //----------------------------------------------------------

    //如果有买入信号
    if (m_sg->shouldBuy(today.datetime)) {
        TradeRecord tr;
        if (m_tm->haveShort(m_stock))
            tr = _buyShort(today, src_today, PART_SIGNAL);
        else
            tr = _buy(today, src_today, PART_SIGNAL);
        return tr.isNull() ? result : tr;
    }

    //发出卖出信号
    if (m_sg->shouldSell(today.datetime)) {
        TradeRecord tr;
        if (m_tm->have(m_stock))
            tr = _sell(today, src_today, PART_SIGNAL);
        else
            tr = _sellShort(today, src_today, PART_SIGNAL);
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
    if (position.number != 0) {
        TradeRecord tr;
        if (src_current_price <= position.stoploss) {
            tr = _sell(today, src_today, PART_STOPLOSS);
            //} else if (current_price >= position.goalPrice) {
        } else if (src_current_price >= _getGoalPrice(today.datetime, src_current_price)) {
            tr = _sell(today, src_today, PART_PROFITGOAL);
        } else {
            price_t current_take_profile = _getTakeProfitPrice(today.datetime);
            if (current_take_profile != 0.0) {
                if (current_take_profile < m_lastTakeProfit) {
                    current_take_profile = m_lastTakeProfit;
                } else {
                    m_lastTakeProfit = current_take_profile;
                }
                if (current_price <= current_take_profile) {
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

    //以当前收盘价为计划价格
    price_t planPrice = src_today.closePrice;

    //计算止损价
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    //如果计划的价格已经小于等于止损价，放弃交易
    if (planPrice <= stoploss) {
        return result;
    }

    //获取可买入数量
    double number = _getBuyNumber(today.datetime, planPrice, planPrice - stoploss, from);
    if (number == 0 || number > m_stock.maxTradeNumber()) {
        return result;
    }

    double min_num = m_stock.minTradeNumber();
    if (min_num > 1) {
        number = number / min_num * min_num;
    }

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    price_t goalPrice = _getGoalPrice(today.datetime, planPrice);
    TradeRecord record =
      m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    if (BUSINESS_BUY != record.business) {
        return result;
    }

    m_lastTakeProfit = _getTakeProfitPrice(record.datetime);
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    return record;
}

TradeRecord System::_buyDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        //无法实际执行，延迟至下一时刻
        _submitBuyRequest(KRecord(today.datetime), KRecord(today.datetime), m_buyRequest.from);
        return result;
    }

    //延迟操作，取当前时刻开盘价
    price_t planPrice = src_today.openPrice;  //取当前时刻的开盘价

    //计算止损价和可买入数量
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        //使用当前计划价格计算止损价和可买入数量
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

    //如果计划买入的价格已经小于等于止损价或者买入数量等于0
    if (planPrice <= stoploss || number == 0) {
        m_buyRequest.clear();
        return result;
    }

    double min_num = m_stock.minTradeNumber();
    if (min_num > 1) {
        number = number / min_num * min_num;
    }

    price_t realPrice = _getRealBuyPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->buy(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                   planPrice, m_buyRequest.from);
    if (BUSINESS_BUY != record.business) {
        m_buyRequest.clear();
        return result;
    }

    m_buy_days = 0;
    m_lastTakeProfit = realPrice;
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyRequest.clear();
    return record;
}

void System::_submitBuyRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_buyRequest.valid) {
        if (m_buyRequest.count > getParam<int>("max_delay_count")) {
            //超出最大延迟次数，清除买入请求
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

TradeRecord System::sellForce(const KRecord& today, const KRecord& src_today, double num,
                              Part from) {
    HKU_ASSERT_M(from == PART_ALLOCATEFUNDS || from == PART_PORTFOLIO,
                 "Only Allocator or Portfolis can perform this operation!");
    TradeRecord result;
    if (getParam<bool>("sell_delay")) {
        if (m_sellRequest.valid) {
            if (m_sellRequest.count > getParam<int>("max_delay_count")) {
                //超出最大延迟次数，清除买入请求
                m_sellRequest.clear();
                return result;
            }
            m_sellRequest.count++;

        } else {
            m_sellRequest.valid = true;
            m_sellRequest.business = BUSINESS_SELL;
            m_sellRequest.count = 1;
        }

        PositionRecord position = m_tm->getPosition(today.datetime, m_stock);
        m_sellRequest.from = from;
        m_sellRequest.datetime = today.datetime;
        m_sellRequest.stoploss = position.stoploss;
        m_sellRequest.goal = position.goalPrice;
        m_sellRequest.number = num;
        return result;

    } else {
        PositionRecord position = m_tm->getPosition(today.datetime, m_stock);
        price_t realPrice = _getRealSellPrice(today.datetime, src_today.closePrice);
        TradeRecord record = m_tm->sell(today.datetime, m_stock, realPrice, num, position.stoploss,
                                        position.goalPrice, src_today.closePrice, from);
        m_trade_list.push_back(record);
        _sellNotifyAll(record);
        return record;
    }
}

TradeRecord System::_sell(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    if (getParam<bool>("sell_delay")) {
        _submitSellRequest(today, src_today, from);
        return result;
    } else {
        return _sellNow(today, src_today, from);
    }
}

TradeRecord System::_sellNow(const KRecord& today, const KRecord& src_today, Part from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;
    double number = 0;

    //计算新的止损价
    price_t stoploss = _getStoplossPrice(today, src_today, today.closePrice);

    //如果新的计划价格已经小于等于新的止损价，则认为需全部卖出
    if (planPrice <= stoploss) {
        number = m_tm->getHoldNumber(today.datetime, m_stock);

        //否则，认为可能只是进行减仓操作
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
        return result;  //卖出操作失败
    }

    //如果已未持仓，最后的止赢价初始为0
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    } else {
        m_lastTakeProfit = _getTakeProfitPrice(today.datetime);
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    return record;
}

TradeRecord System::_sellDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        //无法执行，保留卖出请求，继续延迟至下一时刻
        _submitSellRequest(KRecord(today.datetime), KRecord(today.datetime), m_sellRequest.from);
        return result;
    }

    price_t planPrice = src_today.openPrice;  //取当前时刻的开盘价

    //发出卖出请求时刻的止损价
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
        return result;  //卖出操作失败
    }

    //如果已未持仓，最后的止赢价初始为0
    if (!m_tm->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    }

    m_trade_list.push_back(record);
    _sellNotifyAll(record);
    m_sellRequest.clear();
    return record;
}

void System::_submitSellRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_sellRequest.valid) {
        if (m_sellRequest.count > getParam<int>("max_delay_count")) {
            //超出最大延迟次数，清除买入请求
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
        //无法实际执行，延迟至下一时刻
        //_submitBuyRequest(m_buyRequest.datetime);
        return result;
    }

    price_t planPrice = src_today.closePrice;  //取当前时刻的收盘价

    //取当前时刻的收盘价对应的止损价
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    //确定数量
    double number = _getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number == 0) {
        m_buyShortRequest.clear();
        return result;
    }

    //获取当前空头仓位持有情况
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
    m_lastTakeProfit = realPrice;  //止赢赋值给买入价格
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return record;
}

TradeRecord System::_buyShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice) {
        //无法实际执行，延迟至下一时刻
        //_submitBuyRequest(m_buyRequest.datetime);
        return result;
    }

    price_t planPrice = src_today.openPrice;  //取当前时刻的收盘价

    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        //取当前时刻的收盘价对应的止损价
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

    //获取当前空头仓位持有情况
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
    m_lastTakeProfit = realPrice;  //止赢赋值给买入价格
    m_trade_list.push_back(record);
    _buyNotifyAll(record);
    m_buyShortRequest.clear();
    return result;
}

void System::_submitBuyShortRequest(const KRecord& today, const KRecord& src_today, Part from) {
    if (m_buyShortRequest.valid) {
        if (m_buyShortRequest.count > getParam<int>("max_delay_count")) {
            //超出最大延迟次数，清除买入请求
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
        HKU_WARN("set system param support_borrow_stock to true to short sell");
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
        //当前无法卖出，延迟至下一时刻卖出
        _submitSellShortRequest(today, src_today, from);
        return result;
    }

    price_t planPrice = src_today.closePrice;

    //计算止损价
    price_t stoploss = _getShortStoplossPrice(today, src_today, today.closePrice);

    double number = _getSellShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number == 0) {
        m_sellShortRequest.clear();
        return result;
    }

    price_t goalPrice = _getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = _getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                    planPrice, PART_SIGNAL);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  //卖出操作失败
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
        //无法执行，保留卖出请求，继续延迟至下一时刻
        _submitSellShortRequest(today, src_today, m_sellShortRequest.from);
        return result;
    }

    price_t planPrice = src_today.openPrice;  //取当前时刻的开盘价

    //发出卖出请求时刻的止损价
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

    TradeRecord record = m_tm->sellShort(today.datetime, m_stock, realPrice, number, stoploss, goalPrice,
                                    planPrice, m_sellShortRequest.from);
    if (BUSINESS_SELL_SHORT != record.business) {
        m_sellShortRequest.clear();
        return result;  //卖出操作失败
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
            //超出最大延迟次数，清除买入请求
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
