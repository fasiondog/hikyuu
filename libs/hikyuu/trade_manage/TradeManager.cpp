/*
 * TradeManager.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <fstream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "TradeManager.h"
#include "../trade_sys/system/SystemPart.h"
#include "../utilities/util.h"
#include "../KData.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const TradeManager& tm) {
    os << std::fixed;
    os.precision(4);

    FundsRecord funds = tm.getFunds();
    string strip(",\n");
    os << "TradeManager {\n"
       << "  params: " << tm.getParameter() << strip
       << "  name: " << tm.name() << strip
       << "  init_date: " << tm.initDatetime() << strip
       << "  init_cash: " << tm.initCash() << strip
       << "  firstDatetime: " << tm.firstDatetime() << strip
       << "  lastDatetime: " << tm.lastDatetime() << strip
       << "  brokeLastDatetime: " << tm.getBrokerLastDatetime() << strip
       << "  TradeCostFunc: " << tm.costFunc() << strip
       << "  current cash: " << tm.currentCash() << strip
       << "  current market_value: " << funds.market_value << strip
       << "  current short_market_value: " << funds.short_market_value << strip
       << "  current base_cash: " << funds.base_cash << strip
       << "  current base_asset: " << funds.base_asset << strip
       << "  current borrow_cash: " << funds.borrow_cash << strip
       << "  current borrow_asset: " << funds.borrow_asset << strip
       << "  Position: \n";

    PositionRecordList position = tm.getPositionList();
    PositionRecordList::const_iterator iter = position.begin();
    for (; iter != position.end(); ++iter) {
        os << "    " << iter->number << " " << iter->stock << "\n";
    }

    os << "  Short Position: \n";
    position = tm.getShortPositionList();
    iter = position.begin();
    for (; iter != position.end(); ++iter) {
        os << "    " << iter->number << " " << iter->stock << "\n";
    }

    os << "  Borrow Stock: \n";
    BorrowRecordList borrow = tm.getBorrowStockList();
    BorrowRecordList::const_iterator bor_iter = borrow.begin();
    for (; bor_iter != borrow.end(); ++bor_iter) {
        os << "    " << bor_iter->number << " " << bor_iter->value
              << " " << bor_iter->stock << "\n";
    }

    os   << "}";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}


HKU_API std::ostream & operator<<(std::ostream& os, const TradeManagerPtr& ptm) {
    if (ptm) {
        os << *ptm;
    } else {
        os << "TradeManager(NULL)";
    }
    return os;
}


string TradeManager::toString() const {
    std::stringstream os;
    os << std::fixed;
    os.precision(2);

    FundsRecord funds = getFunds();
    string strip(",\n");
    os << "TradeManager {\n"
       << "  params: " << getParameter() << strip
       << "  name: " << name() << strip
       << "  init_date: " << initDatetime() << strip
       << "  init_cash: " << initCash() << strip
       << "  firstDatetime: " << firstDatetime() << strip
       << "  lastDatetime: " << lastDatetime() << strip
       << "  TradeCostFunc: " << costFunc() << strip
       << "  current cash: " << currentCash() << strip
       << "  current market_value: " << funds.market_value << strip
       << "  current short_market_value: " << funds.short_market_value << strip
       << "  current base_cash: " << funds.base_cash << strip
       << "  current base_asset: " << funds.base_asset << strip
       << "  current borrow_cash: " << funds.borrow_cash << strip
       << "  current borrow_asset: " << funds.borrow_asset << strip
       << "  Position: \n";

    StockManager& sm = StockManager::instance();
    KQuery query(-1);
    PositionRecordList position = getPositionList();
    PositionRecordList::const_iterator iter = position.begin();
    for (; iter != position.end(); ++iter) {
        //os << "    " << iter->number << " " << iter->stock.toString() << "\n";
        price_t invest = iter->buyMoney - iter->sellMoney + iter->totalCost;
        KData k = iter->stock.getKData(query);
        price_t cur_val = k[0].closePrice * iter->number;
        price_t bonus = cur_val - invest;
        DatetimeList date_list = sm.getTradingCalendar(
                KQueryByDate(iter->takeDatetime.date()));
        os << "    " << iter->stock.market_code() << " " << iter->stock.name()
           << " " << iter->takeDatetime << " " << date_list.size()
           << " " << iter->number<< " " << invest
           << " " << cur_val << " " << bonus
           << " " << 100 * bonus / invest
           << "% " << 100 * bonus / m_init_cash << "%\n";
    }

    os << "  Short Position: \n";
    position = getShortPositionList();
    iter = position.begin();
    for (; iter != position.end(); ++iter) {
        os << "    " << iter->number << " " << iter->stock.toString() << "\n";
    }

    os << "  Borrow Stock: \n";
    BorrowRecordList borrow = getBorrowStockList();
    BorrowRecordList::const_iterator bor_iter = borrow.begin();
    for (; bor_iter != borrow.end(); ++bor_iter) {
        os << "    " << bor_iter->number << " " << bor_iter->value
              << " " << bor_iter->stock.toString() << "\n";
    }

    os   << "}";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

TradeManager::TradeManager(const Datetime& datetime, price_t initcash,
        const TradeCostPtr& costfunc, const string& name)
: m_name(name), m_init_datetime(datetime), m_costfunc(costfunc),
  m_checkout_cash(0.0), m_checkin_stock(0.0), m_checkout_stock(0.0),
  m_borrow_cash(0.0) {
    setParam<bool>("reinvest", false);  //红利是否再投资
    setParam<int>("precision", 2);      //计算精度
    setParam<bool>("support_borrow_cash", false);   //是否自动融资
    setParam<bool>("support_borrow_stock", false);  //是否自动融券
    setParam<bool>("save_action", true); //是否保存命令
    m_init_cash = roundEx(initcash, 2);
    m_cash = m_init_cash;
    m_checkin_cash = m_init_cash;
    m_trade_list.push_back(TradeRecord(Null<Stock>(), m_init_datetime,
            BUSINESS_INIT, m_init_cash, m_init_cash, 0.0, 0,
            CostRecord(), 0.0,  m_cash, PART_INVALID));
    m_broker_last_datetime = Datetime::now();
    _saveAction(m_trade_list.back());
}


TradeManager::~TradeManager() {

}


void TradeManager::reset() {
    m_cash     = m_init_cash;
    m_checkin_cash = m_init_cash;
    m_checkout_cash = 0.0;
    m_checkin_stock = 0.0;
    m_checkout_stock = 0.0;
    m_borrow_cash = 0.0;

    m_loan_list.clear();
    m_borrow_stock.clear();

    m_trade_list.clear();
    m_trade_list.push_back(TradeRecord(Null<Stock>(), m_init_datetime,
            BUSINESS_INIT, m_init_cash, m_init_cash, 0.0, 0, CostRecord(),
            0.0,  m_cash,  PART_INVALID));

    m_position.clear();
    m_position_history.clear();
    //m_broker_list
    //m_broker_last_datetime = Datetime::now();
    m_actions.clear();
    _saveAction(m_trade_list.back());
}


TradeManagerPtr TradeManager::clone() {
    TradeManager *p = new TradeManager(m_init_datetime, m_init_cash,
            m_costfunc, m_name);
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_init_datetime = m_init_datetime;
    p->m_init_cash = m_init_cash;

    //costfunc是一个公共的函数对象，是共享实现，无须deepcopy
    p->m_costfunc = m_costfunc;

    p->m_cash = m_cash;
    p->m_checkin_cash = m_checkin_cash;
    p->m_checkout_cash = m_checkout_cash;
    p->m_checkin_stock = m_checkin_stock;
    p->m_checkout_stock = m_checkout_stock;
    p->m_borrow_cash = m_borrow_cash;
    p->m_loan_list = m_loan_list;
    p->m_borrow_stock = m_borrow_stock;
    p->m_trade_list = m_trade_list;
    p->m_position = m_position;
    p->m_position_history = m_position_history;
    p->m_broker_list = m_broker_list;
    p->m_broker_last_datetime = m_broker_last_datetime;

    p->m_actions = m_actions;

    return TradeManagerPtr(p);
}

void TradeManager::regBroker(const OrderBrokerPtr& broker) {
    m_broker_list.push_back(broker);
}

void TradeManager::clearBroker() {
    m_broker_list.clear();
}

double TradeManager::getMarginRate(const Datetime& datetime, const Stock& stock) {
    //TODO 获取保证金比率，默认固定取60%
    return 0.6;
}


Datetime TradeManager::firstDatetime() const {
    Datetime result;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for(; iter != m_trade_list.end(); iter++){
        if( iter->business == BUSINESS_BUY ){
            result = iter->datetime;
            break;
        }
    }
    return result;
}


size_t TradeManager
::getHoldNumber(const Datetime& datetime, const Stock& stock) {
    //日期小于账户建立日期，返回0
    if( datetime < m_init_datetime ){
        return 0;
    }

    //根据权息信息调整持仓数量
    _update(datetime);

    //如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            return pos_iter->second.number;
        }
        return 0;
    }

    //在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    size_t number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for(; iter != m_trade_list.end(); iter++){
        //交易记录中的交易日期已经大于查询日期，则跳出循环
        if( iter->datetime > datetime ){
            break;
        }

        if( iter->stock == stock ){
            if (BUSINESS_BUY == iter->business
                    || BUSINESS_GIFT == iter->business
                    || BUSINESS_CHECKIN_STOCK == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business
                    || BUSINESS_CHECKOUT_STOCK == iter->business) {
                number -= iter->number;

            } else {
                //其他情况忽略
            }
        }
    }
    return number;
}


size_t TradeManager
::getShortHoldNumber(const Datetime& datetime, const Stock& stock) {
    //日期小于账户建立日期，返回0
    if( datetime < m_init_datetime ){
        return 0;
    }

    //根据权息信息调整持仓数量
    _update(datetime);

    //如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_short_position.find(stock.id());
        if (pos_iter != m_short_position.end()) {
            return pos_iter->second.number;
        }
        return 0;
    }

    //在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    size_t number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for(; iter != m_trade_list.end(); iter++){
        //交易记录中的交易日期已经大于查询日期，则跳出循环
        if( iter->datetime > datetime ){
            break;
        }

        if( iter->stock == stock ){
            if (BUSINESS_SELL_SHORT == iter->business) {
                number += iter->number;

            } else if (BUSINESS_BUY_SHORT == iter->business) {
                number -= iter->number;

            } else {
                //其他情况忽略
            }
        }
    }
    return number;
}


size_t TradeManager
::getDebtNumber(const Datetime& datetime, const Stock& stock) {
    if (datetime < m_init_datetime) {
        return 0;
    }

    //根据权息信息调整持仓数量
    _update(datetime);

    if (datetime >= lastDatetime()) {
        borrow_stock_map_type::const_iterator bor_iter;
        bor_iter = m_borrow_stock.find(stock.id());
        if (bor_iter != m_borrow_stock.end()) {
            return bor_iter->second.number;
        }
        return 0;
    }

    size_t debt_n = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        if (iter->datetime > datetime) {
            break;
        }
        if (iter->stock == stock) {
            if (iter->business == BUSINESS_BORROW_STOCK) {
                debt_n += iter->number;
            } else if (iter->business == BUSINESS_RETURN_STOCK) {
                debt_n -= iter->number;
            }
        }
    }
    return debt_n;
}


price_t TradeManager::getDebtCash(const Datetime& datetime) {
    if (datetime < m_init_datetime) {
        return 0.0;
    }

    //根据权息信息调整持仓数量
    _update(datetime);

    if (datetime >= lastDatetime()) {
        return m_borrow_cash;
    }

    price_t debt_cash = 0.0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        if (iter->datetime > datetime) {
            break;
        }
        if (iter->business == BUSINESS_BORROW_CASH) {
            debt_cash += iter->realPrice;
        } else if (iter->business == BUSINESS_RETURN_CASH) {
            debt_cash -= iter->realPrice;
        }
    }
    return debt_cash;
}


TradeRecordList TradeManager
::getTradeList(const Datetime& start_date, const Datetime& end_date) const {
    TradeRecordList result;
    if (start_date >= end_date) {
        return result;
    }

    size_t total = m_trade_list.size();
    if (total == 0) {
        return result;
    }

    TradeRecord temp_record;
    temp_record.datetime = start_date;
    auto low = lower_bound(m_trade_list.begin(),
                           m_trade_list.end(),
                           temp_record,
                           boost::bind(std::less<Datetime>(),
                                       boost::bind(&TradeRecord::datetime, _1),
                                       boost::bind(&TradeRecord::datetime, _2)));

    temp_record.datetime = end_date;
    auto high = lower_bound(m_trade_list.begin(),
                            m_trade_list.end(),
                            temp_record,
                            boost::bind(std::less<Datetime>(),
                                       boost::bind(&TradeRecord::datetime, _1),
                                       boost::bind(&TradeRecord::datetime, _2)));

    result.insert(result.end(), low, high);

    return result;
}


PositionRecordList TradeManager::getPositionList() const {
    PositionRecordList result;
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); iter++){
        result.push_back(iter->second);
    }
    return result;
}


PositionRecordList TradeManager::getShortPositionList() const {
    PositionRecordList result;
    position_map_type::const_iterator iter = m_short_position.begin();
    for (; iter != m_short_position.end(); iter++){
        result.push_back(iter->second);
    }
    return result;
}


PositionRecord TradeManager::getPosition(const Stock& stock) const {
    if (stock.isNull()) {
        return PositionRecord();
    }
    position_map_type::const_iterator iter;
    iter = m_position.find(stock.id());
    if (iter == m_position.end()) {
        return PositionRecord();
    }
    return iter->second;
}


PositionRecord TradeManager::getShortPosition(const Stock& stock) const {
    if (stock.isNull()) {
        return PositionRecord();
    }
    position_map_type::const_iterator iter;
    iter = m_short_position.find(stock.id());
    if (iter == m_short_position.end()) {
        return PositionRecord();
    }
    return iter->second;
}


BorrowRecordList TradeManager::getBorrowStockList() const {
    BorrowRecordList result;
    borrow_stock_map_type::const_iterator iter = m_borrow_stock.begin();
    for (; iter != m_borrow_stock.end(); ++iter) {
        result.push_back(iter->second);
    }
    return result;
}


bool TradeManager::checkin(const Datetime& datetime, price_t cash) {
    if (cash <= 0.0) {
        HKU_ERROR(datetime << " cash(" << cash
                << ") must be > 0! [TradeManager::checkin]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::checkin]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(cash, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime,
            BUSINESS_CHECKIN, in_cash, in_cash, 0.0, 0,
            CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}


bool TradeManager::checkout(const Datetime& datetime, price_t cash) {
    if (cash <= 0.0) {
        HKU_ERROR(datetime << " cash(" << cash
                << ") must be > 0! [TradeManager::checkout]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::checkout]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(cash, precision);
    if (out_cash > m_cash) {
        HKU_ERROR(datetime << " cash(" << cash
                << ") must be <= current cash(" << m_cash
                << ") ! [TradeManager::checkout]");
        return false;
    }

    m_cash = roundEx(m_cash - out_cash, precision);
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime,
            BUSINESS_CHECKOUT, out_cash, out_cash, 0.0, 0,
            CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}


bool TradeManager
::checkinStock(const Datetime& datetime, const Stock& stock,
               price_t price, size_t number) {
    if (stock.isNull()) {
        HKU_ERROR(datetime << " Try checkin Null stock! "
                << " [TradeManager::checkinStock]");
        return false;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code() << " number is zero! "
                << " [TradeManager::checkinStock]");
        return false;
    }

    if (price <= 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " price(" << price << ") must be > 0! "
                        "[TradeManager::checkinStock]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::checkinStock]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    //加入当前持仓
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        m_position[stock.id()] = PositionRecord(stock, datetime,
                Null<Datetime>(), number, 0.0, 0.0, number, market_value,
                0.0, 0.0, 0.0);
    } else {
        PositionRecord& pos = pos_iter->second;
        pos.number += number;
        //pos.stoploss 不变
        pos.totalNumber += number;
        pos.buyMoney = roundEx(pos.buyMoney + market_value, precision);
        //pos.totalCost 不变
        //pos.totalRisk 不变
        //pos.sellMoney 不变
    }

    //加入交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_CHECKIN_STOCK,
            price, price, 0.0, number, CostRecord(), 0.0, m_cash, PART_INVALID));

    //更新累计存入资产价值记录
    m_checkin_stock = roundEx(m_checkin_stock + market_value, precision);

    return true;
}


bool TradeManager
::checkoutStock(const Datetime& datetime, const Stock& stock,
                price_t price, size_t number) {
    if (stock.isNull()) {
        HKU_ERROR(datetime << " Try checkout Null stock! [TradeManager::checkoutStock]");
        return false;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " checkout number is zero! [TradeManager::checkoutStock]");
        return false;
    }

    if (price <= 0.0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << "checkout price(" << price << ") must be > 0.0! "
                        "[TradeManager::checkoutStock] ");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::checkoutStock]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    //当前是否有持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        HKU_ERROR("Try to checkout nonexistent stock! "
                  "[TradeManager::checkoutStock]");
        return false;
    }

    PositionRecord& pos = pos_iter->second;
    //取出数量超出了当前持仓数量
    if (number > pos.number) {
        HKU_ERROR(datetime << " " <<  stock.market_code()
                << " Try to checkout number(" << number
                << ") beyond position number(" << pos.number
                << ")! [TradeManager::checkoutStock]");
        return false;
    }

    int precision = getParam<int>("precision");
    pos.number -= number;
    pos.sellMoney = roundEx(pos.sellMoney + price * number * stock.unit(),
            precision);

    //取出后当前所有持仓数量为0，清除当前持仓，存入历史持仓
    if (0 == pos.number) {
        m_position_history.push_back(pos);
        m_position.erase(stock.id());
    }

    //更新交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime,
            BUSINESS_CHECKOUT_STOCK,price, price, 0.0, number,
            CostRecord(), 0.0, m_cash, PART_INVALID));

    //更新累计取出股票价值
    m_checkout_stock = roundEx(m_checkout_stock - price * number * stock.unit(),
            precision);

    return true;
}


bool TradeManager::borrowCash(const Datetime& datetime, price_t cash) {
    if (cash <= 0.0) {
        HKU_ERROR(datetime << " cash(" << cash
                << ") must be > 0! [TradeManager::borrowCash]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::borrowCash]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(cash, precision);
    CostRecord cost = getBorrowCashCost(datetime, cash);
    m_cash = roundEx(m_cash + in_cash - cost.total, precision);
    m_borrow_cash = roundEx(m_borrow_cash + in_cash, precision);
    m_loan_list.push_back(LoanRecord(datetime, in_cash));
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime,
            BUSINESS_BORROW_CASH, in_cash, in_cash, 0.0, 0,
            cost, 0.0, m_cash, PART_INVALID));
    return true;
}


bool TradeManager::returnCash(const Datetime& datetime, price_t cash) {
    if (cash <= 0.0) {
        HKU_ERROR(datetime << " cash(" << cash << ") must be > 0! "
                "[TradeManager::returnCash]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::returnCash]");
        return false;
    }

    if (m_loan_list.empty()) {
        HKU_ERROR(datetime << " not borrow any cash! [TradeManager::returnCash]");
        return false;
    }

    if (datetime < m_loan_list.back().datetime) {
        HKU_ERROR(datetime << " must be >= the datetime("
                << m_loan_list.back().datetime << ") of last loan record! "
                        "[TradeManager::returnCash]" );
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    int precision = getParam<int>("precision");

    CostRecord cost, cur_cost;
    price_t in_cash = roundEx(cash, precision);
    price_t return_cash = in_cash;
    list<LoanRecord>::iterator iter = m_loan_list.begin();
    for (; iter != m_loan_list.end(); ++iter) {
        if (return_cash <= iter->value) {
            cur_cost = getReturnCashCost(iter->datetime, datetime, return_cash);
            return_cash = 0.0;
        } else { //return_cash > loan.value
            cur_cost = getReturnCashCost(iter->datetime, datetime, iter->value);
            return_cash = roundEx(return_cash - iter->value, precision);
        }

        cost.commission = roundEx(cost.commission + cur_cost.commission, precision);
        cost.stamptax = roundEx(cost.stamptax + cur_cost.stamptax, precision);
        cost.transferfee = roundEx(cost.transferfee + cur_cost.transferfee, precision);
        cost.others = roundEx(cost.others + cur_cost.others, precision);
        cost.total = roundEx(cost.total + cur_cost.total, precision);
        if (return_cash == 0.0)
            break;
    }

    if (return_cash != 0.0) {
        //欲归还的钱多余实际欠款
        HKU_ERROR(datetime << " return cash must <= borrowed cash! "
                "[TradeManager::returnCash]");
        return false;
    }

    price_t out_cash = roundEx(in_cash + cost.total, precision);
    if (out_cash > m_cash) {
        HKU_ERROR(datetime << " cash(" << cash
                << ") must be <= current cash(" << m_cash
                << ")! [TradeManager::returnCash]");
        return false;
    }

    return_cash = in_cash;
    do {
        iter = m_loan_list.begin();
        if (return_cash == iter->value) {
            m_loan_list.pop_front();
            break;
        } else if (return_cash < iter->value) {
            iter->value = roundEx(iter->value - return_cash, precision);
            break;
        } else { //return_cash > iter->value
            return_cash = roundEx(return_cash - iter->value, precision);
            m_loan_list.pop_front();
        }
    } while (!m_loan_list.empty());

    m_cash = roundEx(m_cash - out_cash, precision);
    m_borrow_cash = roundEx(m_borrow_cash - in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime,
            BUSINESS_RETURN_CASH, in_cash, in_cash, 0.0, 0,
            cost, 0.0, m_cash, PART_INVALID));
    return true;
}


bool TradeManager
::borrowStock(const Datetime& datetime, const Stock& stock,
        price_t price, size_t number) {
    if (stock.isNull()) {
        HKU_ERROR(datetime << " Try checkin Null stock! "
                "[TradeManager::borrowStock]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::borrowStock]");
        return false;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Try to borrow number is zero! [TradeManager::borrowStock]");
        return false;
    }

    if (price <= 0.0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " price(" << price
                << ") must be > 0! [TradeManager::borrowStock]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    //加入当前持仓
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    CostRecord cost = getBorrowStockCost(datetime, stock, price, number);

    //更新现金，扣除借入时花费的成本
    m_cash = roundEx(m_cash - cost.total, precision);

    //加入交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_BORROW_STOCK,
            price, price, 0.0, number, cost, 0.0, m_cash, PART_INVALID));

    //更新当前借入股票信息
    borrow_stock_map_type::iterator iter = m_borrow_stock.find(stock.id());
    if (iter == m_borrow_stock.end()) {
        BorrowRecord record(stock, number, market_value);
        BorrowRecord::Data data(datetime, price, number);
        record.record_list.push_back(data);
        m_borrow_stock[stock.id()] = record;
    } else {
        //iter->second.stock = stock;
        iter->second.number += number;
        iter->second.value = roundEx(iter->second.value + market_value, precision);
        BorrowRecord::Data data(datetime, price, number);
        iter->second.record_list.push_back(data);
    }

    return true;
}


bool TradeManager
::returnStock(const Datetime& datetime, const Stock& stock,
        price_t price, size_t number) {
    if (stock.isNull()) {
        HKU_ERROR(datetime << " Try checkout Null stock! "
                "[TradeManager::returnStock]");
        return false;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::returnStock]");
        return false;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " return stock number is zero! [TradeManager::returnStock]");
        return false;
    }

    if (price <= 0.0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " price(" << price
                << ") must be > 0! [TradeManager::returnStock]");
        return false;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    //查询借入股票信息
    borrow_stock_map_type::iterator bor_iter = m_borrow_stock.find(stock.id());
    if (bor_iter == m_borrow_stock.end()) {
        //并未借入股票
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Try to return nonborrowed stock! "
                  "[TradeManager::returnStock]");
        return false;
    }

    BorrowRecord& bor = bor_iter->second;
    if (number > bor.number) {
        //欲归还的数量大于借入的数量
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Try to return number(" << number
                << ") > borrow number(" << bor.number
                << ")! [TradeManager::returnStock");
        return false;
    }

    //更新借入股票信息
    int precision = getParam<int>("precision");
    CostRecord cost, cur_cost;
    price_t market_value = 0.0;
    size_t remain_num = number;
    list<BorrowRecord::Data>::iterator iter = bor.record_list.begin();
    for (; iter != bor.record_list.end(); ++iter) {
        if (remain_num <= iter->number) {
            cur_cost = getReturnStockCost(iter->datetime, datetime, stock, price, number);
            market_value = roundEx(market_value
                    + iter->price * remain_num * stock.unit(), precision);
            remain_num = 0;
        } else { //number > iter->number
            cur_cost = getReturnStockCost(iter->datetime, datetime ,stock, price, number);
            market_value = roundEx(market_value
                    + iter->price * iter->number * stock.unit(), precision);
            remain_num -= iter->number;
        }

        cost.commission = roundEx(cost.commission + cur_cost.commission, precision);
        cost.stamptax = roundEx(cost.stamptax + cur_cost.stamptax, precision);
        cost.transferfee = roundEx(cost.transferfee + cur_cost.transferfee, precision);
        cost.others = roundEx(cost.others + cur_cost.others, precision);
        cost.total = roundEx(cost.total + cur_cost.total, precision);
        if (remain_num == 0)
            break;
    }

    bor.number -= number;
    bor.value = roundEx(bor.value - market_value, precision);

    remain_num = number;
    do {
        iter = bor.record_list.begin();
        if (remain_num == iter->number) {
            bor.record_list.pop_front();
            break;
        } else if (remain_num < iter->number) {
            iter->number -= remain_num;
            break;
        } else { //remain_num > iter->number
            remain_num -=  iter->number;
            bor.record_list.pop_front();
        }
    } while (!bor.record_list.empty());

    if (bor.record_list.empty()) {
        m_borrow_stock.erase(bor_iter);
    }

    //更新现金，扣除归还时花费的成本
    m_cash = roundEx(m_cash - cost.total, precision);

    //更新交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime,
            BUSINESS_RETURN_STOCK, price, price, 0.0, number,
            cost, 0.0, m_cash, PART_INVALID));

    return true;
}


TradeRecord TradeManager::buy(const Datetime& datetime, const Stock& stock,
        price_t realPrice, size_t number, price_t stoploss,
        price_t goalPrice, price_t planPrice, SystemPart from) {
    TradeRecord result;
    result.business = INVALID_BUSINESS;

    if (stock.isNull()) {
        HKU_ERROR(datetime << " Stock is Null! [TradeManager::buy]");
        return result;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::buy]");
        return result;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " numer is zero! [TradeManager::buy]");
        return result;
    }

    if (number < stock.minTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Buy number(" << number
                << ") must be >= minTradeNumber（" << stock.minTradeNumber()
                << ")! [TradeManager::buy]");
        return result;
    }

    if (number > stock.maxTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Buy number(" << number
                << ") must be <= maxTradeNumber(" << stock.maxTradeNumber()
                << ")! [TradeManager::buy]");
        return result;
    }

#if 0  //取消此处的检查，放松限制，另外也可以提高效率。另外，TM只负责交易管理，不许检查
    //检查当日是否存在日线数据，不存在则认为不可交易
    bd::date daydate = datetime.date();
    KRecord krecord = stock.getKRecordByDate(daydate, KQuery::DAY);
    if (krecord == Null<KRecord>()){
        HKU_ERROR(datetime << " " << stock.market_code()
                <<" Non-trading day(" << daydate
                << ") [TradeManager::buy]");
        return result;
    }

    //买入的价格是否在当日最高/最低价范围之内
    if (realPrice > krecord.highPrice || realPrice < krecord.lowPrice) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Invalid buy price(" << realPrice
                << ")! out of highPrice(" << krecord.highPrice
                << ") or lowPrice(" << krecord.lowPrice
                << "! [TradeManager::buy]");
        return result;
    }
#endif

    //根据权息调整当前持仓情况
    _update(datetime);

    CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

    //实际交易需要的现金＝交易数量＊实际交易价格＋交易总成本
    int precision = getParam<int>("precision");
    //price_t money = roundEx(realPrice * number * stock.unit() + cost.total, precision);
    price_t money = roundEx(realPrice * number * stock.unit(), precision);

    if (getParam<bool>("support_borrow_cash")) {
        //获取要求的本金额
        CostRecord bor_cost = getBorrowCashCost(datetime, money);
        double rate = getMarginRate(datetime, stock);
        price_t x = roundEx(m_cash / rate + cost.total + bor_cost.total, precision);
        if (x < money) {
            //能够获得的融资不够，自动追加本金
            checkin(datetime, roundUp(money - x, precision));
        }

        //融资，借入资金
        borrowCash(datetime, roundUp(money, precision));
    }

    if (m_cash < roundEx(money + cost.total, precision)) {
        HKU_WARN(datetime << " " << stock.market_code()
                << " Can't buy, need cash(" << roundEx(money + cost.total, precision)
                << ") > current cash(" << m_cash
                << ")! [TradeManager::buy]");
        return result;
    }

    //更新现金
    m_cash = roundEx(m_cash - money - cost.total, precision);

    //加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice,
                         goalPrice, number, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    //更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        m_position[stock.id()] = PositionRecord(
                stock,
                datetime,
                Null<Datetime>(),
                number,
                stoploss,
                goalPrice,
                number,
                money,
                cost.total,
                roundEx((realPrice - stoploss) * number * stock.unit(), precision),
                0.0);
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += number;
        position.stoploss = stoploss;
        position.goalPrice = goalPrice;
        position.totalNumber += number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.totalCost = roundEx(cost.total + position.totalCost, precision);
        position.totalRisk = roundEx(position.totalRisk +
                (realPrice - stoploss) * number * stock.unit(), precision);
    }

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for(; broker_iter != m_broker_list.end(); ++broker_iter) {
            Datetime realtime = (*broker_iter)->buy(datetime,
                                                    stock.market(),
                                                    stock.code(),
                                                    planPrice, number);
            if (realtime != Null<Datetime>())
                m_broker_last_datetime = realtime;
        }
    }

    _saveAction(result);

    return result;
}


TradeRecord TradeManager::sell(const Datetime& datetime, const Stock& stock,
        price_t realPrice, size_t number, price_t stoploss,
        price_t goalPrice, price_t planPrice, SystemPart from) {
    TradeRecord result;

    if (stock.isNull()) {
        HKU_ERROR(datetime << " Stock is Null! [TradeManager::sell]");
        return result;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::sell]");
        return result;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " number is zero! [TradeManager::sell]");
        return result;
    }

    //对于分红扩股造成不满足最小交易量整数倍的情况，只能通过number=Null<size_t>()的方式全仓卖出
    if (number < stock.minTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Sell number(" << number <<
                ") must be >= minTradeNumber(" << stock.minTradeNumber()
                << ")! [TradeManager::sell]");
        return result;
    }

    if (number != Null<size_t>() && number > stock.maxTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Sell number(" << number
                << ") must be <= maxTradeNumber(" << stock.maxTradeNumber()
                << ")! [TradeManager::sell]");
        return result;
    }

    //未持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        HKU_WARN(datetime << " " << stock.market_code()
                << " This stock was not bought never! ("
                << datetime << realPrice << number << from
                << ") [TradeManager::sell]");
        return result;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    PositionRecord& position = pos_iter->second;

    //调整欲卖出的数量，如果卖出数量等于Null<size_t>()，则表示卖出全部
    size_t real_number = (number == Null<size_t>())
                       ? position.number : number;

    if (position.number < real_number) {
        //欲卖出的数量大于当前持仓的数量
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Try to sell number(" << real_number
                << ") > number of position(" << position.number
                << ")! [Trademanager::sell]");
        return result;
    }

    CostRecord cost = getSellCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    //更新现金余额
    m_cash = roundEx(m_cash + money - cost.total, precision);

    //更新交易记录
    result = TradeRecord(stock, datetime, BUSINESS_SELL, planPrice, realPrice,
            goalPrice, real_number, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    //更新当前持仓情况
    position.number -= real_number;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    //position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_position_history.push_back(position);
        //删除当前持仓
        m_position.erase(stock.id());
    }

    //如果存在借款，则归还
    if (getParam<bool>("support_borrow_cash")
            && m_borrow_cash > 0.0
            && m_cash > 0.0) {
        returnCash(datetime, m_borrow_cash < m_cash ? m_borrow_cash : m_cash);
    }

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for(; broker_iter != m_broker_list.end(); ++broker_iter) {
            Datetime realtime = (*broker_iter)->sell(datetime,
                                                     stock.market(),
                                                     stock.code(),
                                                     planPrice, number);
            m_broker_last_datetime = realtime;
        }
    }

    _saveAction(result);

    return result;
}


TradeRecord TradeManager::sellShort(const Datetime& datetime, const Stock& stock,
        price_t realPrice, size_t number, price_t stoploss,
        price_t goalPrice, price_t planPrice, SystemPart from) {
    TradeRecord result;
    result.business = INVALID_BUSINESS;

    if (stock.isNull()) {
        HKU_ERROR(datetime << " Stock is Null! [TradeManager::sellShort]");
        return result;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::sellShort]");
        return result;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " numer is zero! [TradeManager::sellShort]");
        return result;
    }

    if (number < stock.minTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Buy number(" << number
                << ") must be >= minTradeNumber（" << stock.minTradeNumber()
                << ")! [TradeManager::sellShort]");
        return result;
    }

    if (number > stock.maxTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Buy number(" << number
                << ") must be <= maxTradeNumber(" << stock.maxTradeNumber()
                << ")! [TradeManager::sellShort]");
        return result;
    }

    if (stoploss != 0.0 && stoploss < realPrice) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Sell short's stoploss(" << stoploss
                << ") must be > realPrice(" << realPrice
                <<") or = 0! [TradeManager::sellShort]");
        return result;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    int precision = getParam<int>("precision");

    if (getParam<bool>("support_borrow_stock")) {
        CostRecord cost = getSellCost(datetime, stock, realPrice, number);
        price_t money = roundEx(realPrice * number * stock.unit() + cost.total, precision);
        price_t x = roundEx(m_cash / getMarginRate(datetime, stock), precision);
        if (x < money) {
            checkin(datetime, roundEx(money - x, precision));
        }

        borrowStock(datetime, stock, realPrice, number);
    }

    //判断是否存在已借入的股票及其数量
    borrow_stock_map_type::const_iterator bor_iter;
    bor_iter = m_borrow_stock.find(stock.id());
    if (bor_iter == m_borrow_stock.end()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Non borrowed, can't sell short! [TradeManager::sellShort");
        return result;
    }

    size_t total_borrow_num = bor_iter->second.number;
    size_t can_sell_num = 0;
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    if (pos_iter == m_short_position.end()) {
        //借入的股票并未卖出过
        can_sell_num = total_borrow_num;

    } else {
        //借入的股票已经卖出过
        if (pos_iter->second.number >= total_borrow_num) {
            HKU_ERROR(datetime << " " << stock.market_code()
                    << "Borrowed Stock had all selled! [TradeManager::sellShort]");
            return result;
        }

        //可以卖出的数量 = 借入的总数 - 已卖出的数量
        can_sell_num = total_borrow_num - pos_iter->second.number;
    }

    //如果计划卖出的数量大于可卖出的数量，则已可每卖出的数量卖出
    size_t sell_num = number;
    if (number > can_sell_num) {
        sell_num = can_sell_num;
    }

    CostRecord cost = getSellCost(datetime, stock, realPrice, sell_num);

    price_t money = roundEx(realPrice * sell_num * stock.unit() - cost.total, precision);

    //更新现金
    m_cash = roundEx(m_cash + money, precision);

    //加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_SELL_SHORT, planPrice, realPrice,
                         goalPrice, sell_num, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    //更新当前空头持仓记录
    price_t risk = roundEx((stoploss - realPrice) * sell_num * stock.unit(), precision);

    if (pos_iter == m_short_position.end()) {
        m_short_position[stock.id()] = PositionRecord(
                stock,
                datetime,
                Null<Datetime>(),
                sell_num,
                stoploss,
                goalPrice,
                sell_num,
                cost.total,
                cost.total,
                risk,
                money);
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += sell_num;
        position.stoploss = stoploss;
        position.goalPrice = goalPrice;
        position.totalNumber += sell_num;
        position.buyMoney = roundEx(position.buyMoney + cost.total);
        position.totalCost = roundEx(cost.total + position.totalCost, precision);
        position.totalRisk = roundEx(position.totalRisk + risk, precision);
        position.sellMoney = roundEx(position.sellMoney + money, precision);
    }

    return result;
}


TradeRecord TradeManager::buyShort(const Datetime& datetime, const Stock& stock,
        price_t realPrice, size_t number, price_t stoploss,
        price_t goalPrice, price_t planPrice, SystemPart from) {
    TradeRecord result;

    if (stock.isNull()) {
        HKU_ERROR(datetime << " Stock is Null! [TradeManager::buyShort]");
        return result;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " datetime must be >= lastDatetime("
                << lastDatetime() << ")! [TradeManager::buyShort]");
        return result;
    }

    if (number == 0) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " number is zero! [TradeManager::buyShort]");
        return result;
    }

    if (number < stock.minTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " buyShort number(" << number <<
                ") must be >= minTradeNumber(" << stock.minTradeNumber()
                << ")! [TradeManager::buyShort]");
        return result;
    }

    if (number != Null<size_t>() && number > stock.maxTradeNumber()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " buyShort number(" << number
                << ") must be <= maxTradeNumber(" << stock.maxTradeNumber()
                << ")! [TradeManager::buyShort]");
        return result;
    }

    //未持有空头仓位
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    if (pos_iter == m_short_position.end()) {
        HKU_WARN(datetime << " " << stock.market_code()
                << " This stock was not sell never! [TradeManager::buyShort]");
        return result;
    }

    //根据权息调整当前持仓情况
    _update(datetime);

    PositionRecord& position = pos_iter->second;

    //调整欲买入的数量，如果买入数量等于Null<size_t>()或者大于实际仓位，则表示全部买入
    size_t real_number = (number == Null<size_t>() || number > position.number)
                       ? position.number : number;

    CostRecord cost = getBuyCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    //更新现金余额
    m_cash = roundEx(m_cash - money - cost.total, precision);

    //更新交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY_SHORT, planPrice, realPrice,
            goalPrice, real_number, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    //更新当前空头持仓情况
    position.number -= real_number;
    position.buyMoney = roundEx(position.buyMoney + money + cost.total, precision);
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    //position.sellMoney = roundEx(position.sellMoney, precision);

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_short_position_history.push_back(position);
        //删除当前持仓
        m_short_position.erase(stock.id());
    }

    if (getParam<bool>("support_borrow_stock")) {
        returnStock(datetime, stock, realPrice, real_number);
    }

    return result;
}


price_t TradeManager::cash(const Datetime& datetime, KQuery::KType ktype) {
    FundsRecord funds = getFunds(datetime, ktype);
    return funds.cash;
}


FundsRecord TradeManager
::getFunds(KQuery::KType ktype) const {
    FundsRecord funds;
    int precision = getParam<int>("precision");

    price_t price = 0.0;
    price_t value = 0.0; //当前市值
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        price = record.stock.getMarketValue(lastDatetime(), ktype);
        value = roundEx((value +
                record.number * price * record.stock.unit()), precision);
    }

    price_t short_value = 0.0; //当前空头仓位市值
    iter = m_short_position.begin();
    for (; iter != m_short_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        price = record.stock.getMarketValue(lastDatetime(), ktype);
        short_value = roundEx((short_value +
                record.number * price * record.stock.unit()), precision);
    }
    funds.cash = m_cash;
    funds.market_value = value;
    funds.short_market_value = short_value;
    funds.base_cash = m_checkin_cash - m_checkout_cash;
    funds.base_asset = m_checkin_stock - m_checkout_stock;
    funds.borrow_cash = m_borrow_cash;
    funds.borrow_asset = 0;
    borrow_stock_map_type::const_iterator bor_iter = m_borrow_stock.begin();
    for (; bor_iter != m_borrow_stock.end(); ++bor_iter) {
        funds.borrow_asset += bor_iter->second.value;
    }
    return funds;
}

FundsRecord TradeManager
::getFunds(const Datetime& indatetime, KQuery::KType ktype) {
    FundsRecord funds;
    int precision = getParam<int>("precision");

    //datetime为Null时，直接返回当前账户中的现金和买入时占用的资金，以及累计存取资金
    if (indatetime == Null<Datetime>()
            || indatetime == lastDatetime()) {
        return getFunds(ktype);
    }

    Datetime datetime(indatetime.year(),indatetime.month(), indatetime.day(), 11, 59);
    price_t market_value = 0.0;
    price_t short_market_value = 0.0;
    if (datetime > lastDatetime()) {
        //根据权息数据调整持仓
        _update(datetime);

        //查询日期大于等于最后交易日期时，直接计算当前持仓证券的市值
        position_map_type::const_iterator iter = m_position.begin();
        for (; iter != m_position.end(); ++iter) {
            price_t price = iter->second.stock.getMarketValue(datetime, ktype);
            market_value = roundEx(market_value
                    + price * iter->second.number * iter->second.stock.unit(),
                    precision);
        }

        iter = m_short_position.begin();
        for (; iter != m_short_position.end(); ++iter) {
            price_t price = iter->second.stock.getMarketValue(datetime, ktype);
            short_market_value = roundEx(short_market_value
                    + price * iter->second.number * iter->second.stock.unit(),
                    precision);
        }

        funds.cash = m_cash;
        funds.market_value = market_value;
        funds.short_market_value = short_market_value;
        funds.base_cash = m_checkin_cash - m_checkout_cash;
        funds.base_asset = m_checkin_stock - m_checkout_stock;
        funds.borrow_cash = m_borrow_cash;
        funds.borrow_asset = 0;
        borrow_stock_map_type::iterator bor_iter = m_borrow_stock.begin();
        for (; bor_iter != m_borrow_stock.end(); ++bor_iter) {
            funds.borrow_asset += bor_iter->second.value;
        }

        return funds;
    } // if datetime >= lastDatetime()


    //当查询日期小于最后交易日期时，遍历交易记录，计算当日的市值和现金
    price_t cash = m_init_cash;
    struct Stock_Number {
        Stock_Number(): number(0) {}
        Stock_Number(const Stock& stock, size_t number)
        : stock(stock), number(number) {}

        Stock stock;
        size_t number;
    };

    price_t checkin_cash = 0.0;
    price_t checkout_cash = 0.0;
    price_t checkin_stock = 0.0;
    price_t checkout_stock = 0.0;
    map<hku_uint64, Stock_Number> stock_map;
    map<hku_uint64, Stock_Number> short_stock_map;
    map<hku_uint64, Stock_Number>::iterator stock_iter;
    map<hku_uint64, Stock_Number>::iterator short_stock_iter;
    map<hku_uint64, BorrowRecord> bor_stock_map;
    map<hku_uint64, BorrowRecord>::iterator bor_stock_iter;

    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        if (iter->datetime > datetime) {
            //如果交易记录的日期大于指定的日期则跳出循环，处理完毕
            break;
        }

        cash = iter->cash;
        switch (iter->business) {
        case BUSINESS_INIT:
            checkin_cash += iter->realPrice;
            break;

        case BUSINESS_BUY:
        case BUSINESS_GIFT:
            stock_iter = stock_map.find(iter->stock.id());
            if (stock_iter != stock_map.end()) {
                stock_iter->second.number += iter->number;
            } else {
                stock_map[iter->stock.id()] = Stock_Number(iter->stock,
                        iter->number);
            }
            break;

        case BUSINESS_SELL:
            stock_iter = stock_map.find(iter->stock.id());
            if (stock_iter != stock_map.end()) {
                stock_iter->second.number -= iter->number;
            } else {
                HKU_WARN(datetime << " " << iter->stock.market_code()
                        << " Sell error in m_trade_list! [TradeManager::getFunds]" );
            }
            break;

        case BUSINESS_SELL_SHORT:
            short_stock_iter = short_stock_map.find(iter->stock.id());
            if (short_stock_iter != short_stock_map.end()) {
                short_stock_iter->second.number += iter->number;
            } else {
                short_stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
            }
            break;

        case BUSINESS_BUY_SHORT:
            short_stock_iter = short_stock_map.find(iter->stock.id());
            if (short_stock_iter != short_stock_map.end()) {
                short_stock_iter->second.number -= iter->number;
            } else {
                HKU_WARN(datetime << " " << iter->stock.market_code()
                        << " BuyShort Error in m_trade_list! [TradeManager::getFunds");
            }
            break;

        case BUSINESS_BONUS:
            break;

        case BUSINESS_CHECKIN:
            checkin_cash += iter->realPrice;
            break;

        case BUSINESS_CHECKOUT:
            checkout_cash += iter->realPrice;
            break;

        case BUSINESS_CHECKIN_STOCK:
            stock_iter = stock_map.find(iter->stock.id());
            if (stock_iter != stock_map.end()) {
                stock_map[iter->stock.id()].number += iter->number;
            } else {
                stock_map[iter->stock.id()] = Stock_Number(iter->stock,
                        iter->number);
            }
            checkin_stock = roundEx(checkin_stock
                    + iter->realPrice * iter->number * iter->stock.unit(),
                    precision);
            break;

        case BUSINESS_CHECKOUT_STOCK:
            stock_iter = stock_map.find(iter->stock.id());
            if (stock_iter != stock_map.end()) {
                stock_map[iter->stock.id()].number -= iter->number;
            } else {
                HKU_WARN(datetime << " " << iter->stock.market_code()
                        <<" CheckoutStock Error in m_trade_list! [TradeManager::getFunds]" );
            }
            checkout_stock = roundEx(checkout_stock
                    + iter->realPrice * iter->number * iter->stock.unit(),
                    precision);
            break;

        case BUSINESS_BORROW_CASH:
            funds.borrow_cash += iter->realPrice;
            break;

        case BUSINESS_RETURN_CASH:
            funds.borrow_cash -= iter->realPrice;
            break;

        case BUSINESS_BORROW_STOCK:
            funds.borrow_asset = roundEx(funds.borrow_asset
                    + iter->realPrice * iter->number * iter->stock.unit(),
                    precision);
            bor_stock_iter = bor_stock_map.find(iter->stock.id());
            if (bor_stock_iter == bor_stock_map.end()) {
                BorrowRecord bor;
                BorrowRecord::Data data(iter->datetime, iter->realPrice, iter->number);
                bor.record_list.push_back(data);
                bor_stock_map[iter->stock.id()] = bor;
            } else {
                BorrowRecord::Data data(iter->datetime, iter->realPrice, iter->number);
                bor_stock_iter->second.record_list.push_back(data);
            }
            break;

        case BUSINESS_RETURN_STOCK:
            bor_stock_iter = bor_stock_map.find(iter->stock.id());
            if (bor_stock_iter == bor_stock_map.end()) {
                HKU_WARN(iter->datetime << " " << iter->stock.market_code()
                        << " Error return stock in m_trade_list! TradeManager::getFunds]");

            } else {
                BorrowRecord& bor = bor_stock_iter->second;
                list<BorrowRecord::Data>::iterator bor_iter = bor.record_list.begin();
                size_t remain_num = iter->number;
                do {
                    bor_iter = bor.record_list.begin();
                    if (remain_num == bor_iter->number) {
                        funds.borrow_asset -= roundEx(bor_iter->price
                           * remain_num * iter->stock.unit(), precision);
                        bor.record_list.pop_front();
                        break;

                    } else if (remain_num < bor_iter->number) {
                        funds.borrow_asset -= roundEx(bor_iter->price
                           * remain_num * iter->stock.unit(), precision);
                        bor_iter->number -= remain_num;
                        break;

                    } else { //remain_num > bor_iter->number
                        funds.borrow_asset -= roundEx(bor_iter->price
                           * bor_iter->number * iter->stock.unit(), precision);
                        remain_num -= bor_iter->number;
                        bor.record_list.pop_front();
                    }
                } while (!bor.record_list.empty());

                if (bor.record_list.empty()) {
                    bor_stock_map.erase(bor_stock_iter);
                }
            }

            break;

        default:
            HKU_WARN(datetime << " " << iter->stock.market_code()
                    << "Unknow business in m_trade_list! [TradeManager::getFunds]");
            break;
        }
    }

    stock_iter = stock_map.begin();
    for (; stock_iter != stock_map.end(); ++stock_iter) {
        const size_t& number = stock_iter->second.number;
        if (number == 0) {
            continue;
        }

        price_t price = stock_iter->second.stock.getMarketValue(datetime, ktype);
        market_value = roundEx(market_value + price * number * stock_iter->second.stock.unit(),
                               precision);
    }

    short_stock_iter = short_stock_map.begin();
    for (; short_stock_iter != short_stock_map.end(); ++short_stock_iter) {
        const size_t& number = short_stock_iter->second.number;
        if (number == 0) {
            continue;
        }

        price_t price = short_stock_iter->second.stock.getMarketValue(datetime, ktype);
        short_market_value = roundEx(short_market_value + price * number * stock_iter->second.stock.unit(),
                               precision);
    }

    funds.cash = cash;
    funds.market_value = market_value;
    funds.short_market_value = short_market_value;
    funds.base_cash = checkin_cash - checkout_cash;
    funds.base_asset = checkin_stock - checkout_stock;
    return funds;
}


PriceList TradeManager
::getFundsCurve(const DatetimeList& dates, KQuery::KType ktype) {
    size_t total = dates.size();
    PriceList result(total);
    int precision = getParam<int>("precision");
    for (size_t i = 0; i < total; ++i) {
        FundsRecord funds = getFunds(dates[i], ktype);
        result[i] = roundEx(funds.cash + funds.market_value - funds.borrow_cash
                  - funds.borrow_asset, precision);
    }
    return result;
}


PriceList TradeManager
::getProfitCurve(const DatetimeList& dates, KQuery::KType ktype) {
    size_t total = dates.size();
    PriceList result(total);
    if (total == 0)
        return result;

    size_t i = 0;
    while (dates[i] < m_init_datetime && i < total) {
        result[i] = 0;
        i++;
    }
    int precision = getParam<int>("precision");
    for (; i < total; ++i) {
        FundsRecord funds = getFunds(dates[i], ktype);
        result[i] = roundEx(funds.cash + funds.market_value - funds.borrow_cash
                  - funds.borrow_asset - funds.base_cash - funds.base_asset,
                  precision);
    }

    return result;
}


/******************************************************************************
 *  每次执行交易操作时，先根据权息信息调整持有仓位及现金记录
 *  采用滞后更新的策略，即只在需要获取当前持仓情况及卖出时更新当前的持仓及资产情况
 *  输入参数： 本次操作的日期
 *  历史记录： 1) 2009/12/22 added
 *****************************************************************************/
void TradeManager::_update(const Datetime& datetime){
    if(!getParam<bool>("reinvest")){
        return;
    }

    if (datetime < lastDatetime()) {
        HKU_ERROR(datetime << " update datetime should be < lastDatetime("
                << lastDatetime() << ")! [TradeManager::_update]");
        return;
    }

    //权息信息查询日期范围
    bd::date start_date = lastDatetime().date() + bd::days(1);
    bd::date end_date = datetime.date() + bd::days(1);

    int precision = getParam<int>("precision");
    price_t total_bonus = 0.0;
    price_t last_cash  = m_cash;
    TradeRecordList new_trade_buffer;

    //更新持仓信息，并缓存新增的交易记录
    position_map_type::iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter){
        PositionRecord& position = position_iter->second;
        Stock stock = position.stock;

        StockWeightList weights = stock.getWeight(start_date, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for(; weight_iter != weights.end(); ++weight_iter){
            //如果没有红利并且也（派股和转增股数量都为零），则跳过
            if( 0.0 == weight_iter->bonus()
                    && 0.0 == weight_iter->countAsGift()
                    && 0.0 == weight_iter->increasement() ){
                continue;
            }

            //必须在加入配送股之前，因为配送股会引起持仓数量的变化
            if( weight_iter->bonus() != 0.0 ){
                price_t bonus = roundEx(
                        position.number * weight_iter->bonus() * 0.1,
                        precision);
                position.sellMoney += bonus;
                last_cash += bonus;
                total_bonus += bonus;
                m_cash += bonus;

                TradeRecord record(stock,weight_iter->datetime(),
                        BUSINESS_BONUS, bonus, bonus, 0.0, 0,
                        CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            size_t addcount = (size_t)((position.number / 10) *
                    (weight_iter->countAsGift() + weight_iter->increasement()));
            if (addcount != 0) {
                position.number += addcount;
                position.totalNumber += addcount;
                TradeRecord record(stock, weight_iter->datetime(),
                        BUSINESS_GIFT, 0.0, 0.0, 0.0, addcount,
                        CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }
        } /* for weight */
    } /* for position */

    std::sort(new_trade_buffer.begin(), new_trade_buffer.end(),
            boost::bind(std::less<Datetime>(),
                    boost::bind(&TradeRecord::datetime, _1),
                    boost::bind(&TradeRecord::datetime, _2)));

    size_t total = new_trade_buffer.size();
    for (size_t i = 0; i < total; ++i) {
        if (new_trade_buffer[i].business == BUSINESS_BONUS) {
            price_t bonus = new_trade_buffer[i].realPrice;
            for (size_t j = i; j < total; ++j) {
                new_trade_buffer[j].cash += bonus;
            }
        }
    }

    for (size_t i = 0; i < total; ++i) {
        m_trade_list.push_back(new_trade_buffer[i]);
    }
}


void TradeManager::_saveAction(const TradeRecord& record) {
    if (getParam<bool>("save_action") == false)
        return;

    std::stringstream buf(std::stringstream::out);
    string my_tm("td = my_tm.");
    string sep(", ");
    switch (record.business) {
    case BUSINESS_INIT:
        buf << "my_tm = crtTM(datetime=Datetime('"
            << record.datetime.toString() << "'), "
            << "initCash=" << record.cash << sep
            << "costFunc=" << m_costfunc->name() << "("
            << m_costfunc->getParameter().getNameValueList() << "), "
            << "name='" << m_name << "'"
            << ")";
        break;

    case BUSINESS_CHECKIN:
        buf << my_tm
            << "checkin(Datetime('" << record.datetime.toString() << "'), "
            << record.cash
            << ")";
        break;

    case BUSINESS_CHECKOUT:
        buf << my_tm
            << "checkout(Datetime('" << record.datetime.toString() << "'), "
            << record.cash
            << ")";
        break;

    case BUSINESS_BUY:
        buf << my_tm
            << "buy(Datetime('" << record.datetime.toString() << "'), "
            << "sm['" << record.stock.market_code() << "'], "
            << record.realPrice << sep
            << record.number << sep
            << record.stoploss << sep
            << record.goalPrice << sep
            << record.planPrice << sep
            << record.from
            << ")";
        break;

    case BUSINESS_SELL:
        buf << my_tm
            << "sell(Datetime('" << record.datetime.toString() << "'),"
            << "sm['" << record.stock.market_code() << "'], "
            << record.realPrice << sep
            << record.number << sep
            << record.stoploss << sep
            << record.goalPrice << sep
            << record.planPrice << sep
            << record.from
            << ")";
        break;

    default:
        break;
    }

    m_actions.push_back(buf.str());
}


void TradeManager::tocsv(const string& path) {
    string filename1, filename2, filename3, filename4, filename5;
    if( m_name.empty() ){
        string date = m_init_datetime.toString();
        filename1 = path + "/" + date + "_交易记录.csv";
        filename2 = path + "/" + date + "_已平仓记录.csv";
        filename3 = path + "/" + date + "_未平仓记录.csv";
        filename4 = path + "/" + date + "_资产净值.csv";
        filename5 = path + "/" + date + "_actions.txt";
    }else{
        filename1 = path + "/" + m_name + "_交易记录.csv";
        filename2 = path + "/" + m_name + "_已平仓记录.csv";
        filename3 = path + "/" + m_name + "_未平仓记录.csv";
        filename4 = path + "/" + m_name + "_资产净值.csv";
        filename5 = path + "/" + m_name + "_actions.txt";
    }

    string sep(",");

    //导出交易记录
    std::ofstream file(filename1.c_str());
    if (!file) {
        HKU_ERROR("Can't create file(" << filename1 << ")! [TradeManager::tocvs");
        return;
    }

    file.setf(std::ios_base::fixed);
    file.precision(3);
    file << "#成交日期,证券代码,证券名称,业务名称,计划交易价格,"
            "实际成交价格,目标价格,成交数量,佣金,印花税,过户费,其他成本,交易总成本,"
            "止损价,现金余额,信号来源,日期,开盘价,最高价,最低价,收盘价,"
            "成交金额,成交量" << std::endl;
    TradeRecordList::const_iterator trade_iter = m_trade_list.begin();
    for(; trade_iter != m_trade_list.end(); ++trade_iter){
        const TradeRecord& record = *trade_iter;
        if( record.stock.isNull() ){
            file << record.datetime << sep << sep << sep
                 << getBusinessName(record.business) << sep
                 << record.planPrice << sep
                 << record.realPrice << sep
                 << record.goalPrice << sep
                 << record.number << sep
                 << record.cost.commission << sep
                 << record.cost.stamptax << sep
                 << record.cost.transferfee << sep
                 << record.cost.others << sep
                 << record.cost.total << sep
                 << record.stoploss << sep
                 << record.cash << sep
                 << getSystemPartName(record.from)
                 << std::endl;
        }else{
            file << record.datetime << sep
                 << record.stock.market_code() << sep
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
                 << utf8_to_gb(record.stock.name()) << sep
#else
                 << record.stock.name() << sep
#endif
                 << getBusinessName(record.business) << sep
                 << record.planPrice << sep
                 << record.realPrice << sep
                 << record.goalPrice << sep
                 << record.number << sep
                 << record.cost.commission << sep
                 << record.cost.stamptax << sep
                 << record.cost.transferfee << sep
                 << record.cost.others << sep
                 << record.cost.total << sep
                 << record.stoploss << sep
                 << record.cash << sep
                 << getSystemPartName(record.from) << sep;
            if (BUSINESS_BUY == record.business
                    || BUSINESS_SELL == record.business) {
                KRecord kdata = record.stock.getKRecordByDate(record.datetime,
                                                              KQuery::DAY);
                if (kdata.isValid()) {
                    file << kdata.datetime << sep
                         << kdata.openPrice << sep
                         << kdata.highPrice << sep
                         << kdata.lowPrice << sep
                         << kdata.closePrice << sep
                         << kdata.transAmount << sep
                         << kdata.transCount;
                }
            }
            file << std::endl;
        }
    }
    file.close();

    //导出已平仓记录
    file.open(filename2.c_str());
    if (!file) {
        HKU_ERROR("Can't create file! [TradeManager::tocvs");
        return;
    }

    file << "#建仓日期,平仓日期,证券代码,证券名称,累计持仓数量,"
            "累计花费资金,累计交易成本,已转化资金,总盈利,累积风险" << std::endl;
    PositionRecordList::const_iterator history_iter = m_position_history.begin();
    for (; history_iter != m_position_history.end(); ++history_iter) {
        const PositionRecord& record = *history_iter;
        file << record.takeDatetime << sep
             << record.cleanDatetime << sep
             << record.stock.market_code() << sep
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
             << utf8_to_gb(record.stock.name()) << sep
#else
             << record.stock.name() << sep
#endif
             << record.totalNumber << sep
             << record.buyMoney << sep
             << record.totalCost << sep
             << record.sellMoney << sep
             << record.sellMoney - record.totalCost - record.buyMoney << sep
             << record.totalRisk << std::endl;
    }
    file.close();

    //导出未平仓记录
    file.open(filename3.c_str());
    if (!file) {
        HKU_ERROR("Can't create file! [TradeManager::tocvs");
        return;
    }

    file << "#建仓日期,平仓日期,证券代码,证券名称,当前持仓数量,累计持仓数量,"
            "累计花费资金,累计交易成本,已转化资金,累积风险,"
            "累计浮动盈亏,当前盈亏成本价" << std::endl;
    position_map_type::const_iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        const PositionRecord& record = position_iter->second;
        file << record.takeDatetime << sep
             << record.cleanDatetime << sep
             << record.stock.market_code() << sep
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
             << utf8_to_gb(record.stock.name()) << sep
#else
             << record.stock.name() << sep
#endif
             << record.number << sep
             << record.totalNumber << sep
             << record.buyMoney << sep
             << record.totalCost << sep
             << record.sellMoney << sep
             << record.totalRisk << sep;
        size_t pos = record.stock.getCount(KQuery::DAY);
        if (pos != 0) {
            KRecord krecord = record.stock.getKRecord(pos - 1, KQuery::DAY);
            price_t bonus = record.buyMoney - record.sellMoney - record.totalCost;
            file << record.number * krecord.closePrice - bonus << sep
                 << bonus / record.number << std::endl;
        }
    }
    file.close();

    //到处执行命令
    //导出已平仓记录
    file.open(filename5.c_str());
    if (!file) {
        HKU_ERROR("Can't create file! [TradeManager::tocvs");
        return;
    }

    list<string>::const_iterator action_iter = m_actions.begin();
    for (; action_iter != m_actions.end(); ++action_iter) {
        file << *action_iter << std::endl;
    }
    file.close();
}


bool TradeManager::addTradeRecord(const TradeRecord& tr) {
    string func_name = " [TradeManager::addTradeRecord]";

    if (BUSINESS_INIT == tr.business) {
        return _add_init_tr(tr);
    }

    if (tr.datetime < lastDatetime()) {
        HKU_ERROR("tr.datetime must be >= lastDatetime("
                << lastDatetime() << ")!" << func_name);
        return false;
    }

    switch(tr.business) {
    case BUSINESS_INIT:
        return false;

    case BUSINESS_BUY:
        return _add_buy_tr(tr);

    case BUSINESS_SELL:
        return _add_sell_tr(tr);

    case BUSINESS_GIFT:
        return _add_gift_tr(tr);

    case BUSINESS_BONUS:
        return _add_bonus_tr(tr);

    case BUSINESS_CHECKIN:
        return _add_checkin_tr(tr);

    case BUSINESS_CHECKOUT:
        return _add_checkout_tr(tr);

    case BUSINESS_CHECKIN_STOCK:
        return _add_checkin_stock_tr(tr);

    case BUSINESS_CHECKOUT_STOCK:
        return _add_checkout_stock_tr(tr);

    case BUSINESS_BORROW_CASH:
        return _add_borrow_cash_tr(tr);

    case BUSINESS_RETURN_CASH:
        return _add_return_cash_tr(tr);

    case BUSINESS_BORROW_STOCK:
        return _add_borrow_stock_tr(tr);

    case BUSINESS_RETURN_STOCK:
        return _add_return_stock_tr(tr);

    case BUSINESS_SELL_SHORT:
        return _add_sell_short_tr(tr);

    case BUSINESS_BUY_SHORT:
        return _add_buy_short_tr(tr);

    case INVALID_BUSINESS:
    default:
        HKU_ERROR("tr.business is invalid(" << tr.business << ")!" << func_name);
        return false;
    }

    return false;
}

bool TradeManager::_add_init_tr(const TradeRecord& tr) {
    assert(BUSINESS_INIT == tr.business);

    m_init_datetime = tr.datetime;
    m_init_cash = roundEx(tr.realPrice, getParam<int>("precision"));
    reset();

    return true;
}

bool TradeManager::_add_buy_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_buy_tr]");

    if (tr.stock.isNull()) {
        HKU_ERROR("tr.stock is null!" << func_name);
        return false;
    }

    if (tr.number == 0) {
        HKU_ERROR("tr.number is zero!" << func_name);
        return false;
    }

    if (tr.number < tr.stock.minTradeNumber()
     || tr.number > tr.stock.maxTradeNumber()) {
        HKU_ERROR("tr.number out of range!" << func_name);
        return false;
    }

    int precision = getParam<int>("precision");
    TradeRecord new_tr(tr);
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    if (m_cash < roundEx(money + tr.cost.total, precision)) {
        HKU_WARN("Don't have enough money!" << func_name);
        return false;
    }

    m_cash = roundEx(m_cash - money - tr.cost.total, precision);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    //更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        m_position[tr.stock.id()] = PositionRecord(
                tr.stock,
                tr.datetime,
                Null<Datetime>(),
                tr.number,
                tr.stoploss,
                tr.goalPrice,
                tr.number,
                money,
                tr.cost.total,
                roundEx((tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(), precision),
                0.0);
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += tr.number;
        position.stoploss = tr.stoploss;
        position.goalPrice = tr.goalPrice;
        position.totalNumber += tr.number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.totalCost = roundEx(tr.cost.total + position.totalCost, precision);
        position.totalRisk = roundEx(position.totalRisk +
                (tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(), precision);
    }

    _saveAction(new_tr);

    return true;
}

bool TradeManager::_add_sell_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_sell_tr]");

    if (tr.stock.isNull()) {
        HKU_ERROR("tr.stock is Null!" << func_name);
        return false;
    }

    if (tr.number == 0) {
        HKU_ERROR("tr.number is zero!" << func_name);
        return false;
    }

    //未持仓
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        HKU_ERROR("No position!" << func_name);
        return false;
    }

    PositionRecord& position = pos_iter->second;

    if (position.number < tr.number) {
        //欲卖出的数量大于当前持仓的数量
        HKU_ERROR("Try sell number greater position!" << func_name);
        return false;
    }

    int precision = getParam<int>("precision");
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    //更新现金余额
    m_cash = roundEx(m_cash + money - tr.cost.total, precision);


    //更新交易记录
    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    //更新当前持仓情况
    position.number -= tr.number;
    position.stoploss = tr.stoploss;
    position.goalPrice = tr.goalPrice;
    //position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + tr.cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);

    if (position.number == 0) {
        position.cleanDatetime = tr.datetime;
        m_position_history.push_back(position);
        //删除当前持仓
        m_position.erase(tr.stock.id());
    }

    _saveAction(new_tr);

    return true;
}

bool TradeManager::_add_gift_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_gift_tr]");

    if (tr.stock.isNull()) {
        HKU_ERROR("tr.stock is null!" << func_name);
        return false;
    }

    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        HKU_ERROR("No position!" << func_name);
        return false;
    }

    PositionRecord& position = pos_iter->second;
    position.number += tr.number;
    position.totalNumber += tr.number;

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_bonus_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_bonus_tr]");

    if (tr.stock.isNull()) {
        HKU_ERROR("tr.stock is null!" << func_name);
        return false;
    }

    if (tr.realPrice <= 0.0) {
        HKU_ERROR("tr.realPrice <= 0.0!" << func_name);
        return false;
    }

    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        HKU_ERROR("No position!" << func_name);
        return false;
    }

    PositionRecord& position = pos_iter->second;
    position.sellMoney += tr.realPrice;
    m_cash += tr.realPrice;

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_checkin_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_checkin_tr]");
    if (tr.realPrice <= 0.0) {
        HKU_ERROR("tr.realPrice <= 0.0!" << func_name);
        return false;
    }

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(tr.realPrice, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), tr.datetime,
            BUSINESS_CHECKIN, in_cash, in_cash, 0.0, 0,
            CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::_add_checkout_tr(const TradeRecord& tr) {
    string func_name(" [TradeManager::_add_checkout_tr]");

    if (tr.realPrice <= 0.0) {
        HKU_ERROR("tr.realPrice <= 0.0!" << func_name);
        return false;
    }

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(tr.realPrice, precision);
    if (out_cash > m_cash) {
        HKU_ERROR("Checkout money > current cash!" << func_name);
        return false;
    }

    m_cash = roundEx(m_cash - out_cash, precision);
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), tr.datetime,
            BUSINESS_CHECKOUT, out_cash, out_cash, 0.0, 0,
            CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::_add_checkin_stock_tr(const TradeRecord& tr) {
    //TODO: TradeManager::_add_checkin_stock_tr
    return false;
}

bool TradeManager::_add_checkout_stock_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_borrow_cash_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_return_cash_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_borrow_stock_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_return_stock_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_sell_short_tr(const TradeRecord& tr) {
    return false;
}

bool TradeManager::_add_buy_short_tr(const TradeRecord& tr) {
    return false;
}


} /* namespace hku */
