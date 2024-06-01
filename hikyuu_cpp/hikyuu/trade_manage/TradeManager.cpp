/*
 * TradeManager.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <fstream>
#include <sstream>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "TradeManager.h"
#include "../trade_sys/system/SystemPart.h"
#include "../KData.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TradeManager)
#endif

namespace hku {

string TradeManager::str() const {
    std::stringstream os;
    os << std::fixed;
    os.precision(2);

    FundsRecord funds = getFunds();
    string strip(",\n");
    os << "TradeManager {\n"
       << "  params: " << getParameter() << strip << "  name: " << name() << strip
       << "  init_date: " << initDatetime() << strip << "  init_cash: " << initCash() << strip
       << "  firstDatetime: " << firstDatetime() << strip << "  lastDatetime: " << lastDatetime()
       << strip << "  TradeCostFunc: " << costFunc() << strip << "  current total funds: "
       << funds.cash + funds.market_value + funds.borrow_asset - funds.short_market_value << strip
       << "  current cash: " << currentCash() << strip
       << "  current market_value: " << funds.market_value << strip
       << "  current short_market_value: " << funds.short_market_value << strip
       << "  current base_cash: " << funds.base_cash << strip
       << "  current base_asset: " << funds.base_asset << strip
       << "  current borrow_cash: " << funds.borrow_cash << strip
       << "  current borrow_asset: " << funds.borrow_asset << strip << "  Position: \n";

    StockManager& sm = StockManager::instance();
    KQuery query(-1);
    PositionRecordList position = getPositionList();
    PositionRecordList::const_iterator iter = position.begin();
    for (; iter != position.end(); ++iter) {
        price_t invest = iter->buyMoney - iter->sellMoney + iter->totalCost;
        KData k = iter->stock.getKData(query);
        price_t cur_val = k[0].closePrice * iter->number;
        price_t bonus = cur_val - invest;
        DatetimeList date_list =
          sm.getTradingCalendar(KQueryByDate(Datetime(iter->takeDatetime.date())));
        os << "    " << iter->stock.market_code() << " " << iter->stock.name() << " "
           << iter->takeDatetime << " " << date_list.size() << " " << iter->number << " " << invest
           << " " << cur_val << " " << bonus << " " << 100 * bonus / invest << "% "
           << 100 * bonus / m_init_cash << "%\n";
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
        os << "    " << bor_iter->number << " " << bor_iter->value << " "
           << bor_iter->stock.toString() << "\n";
    }

    os << "}";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

TradeManager::TradeManager(const Datetime& datetime, price_t initcash, const TradeCostPtr& costfunc,
                           const string& name)
: TradeManagerBase(name, costfunc),
  m_init_datetime(datetime),
  m_last_update_datetime(datetime),
  m_checkout_cash(0.0),
  m_checkin_stock(0.0),
  m_checkout_stock(0.0),
  m_borrow_cash(0.0) {
    setParam<bool>("support_borrow_cash", false);   // 是否自动融资
    setParam<bool>("support_borrow_stock", false);  // 是否自动融券
    setParam<bool>("save_action", true);            // 是否保存命令
    m_init_cash = roundEx(initcash, 2);
    m_cash = m_init_cash;
    m_checkin_cash = m_init_cash;
    m_trade_list.push_back(TradeRecord(Null<Stock>(), m_init_datetime, BUSINESS_INIT, m_init_cash,
                                       m_init_cash, 0.0, 0, CostRecord(), 0.0, m_cash,
                                       PART_INVALID));
    m_broker_last_datetime = Datetime::now();
    _saveAction(m_trade_list.back());
}

TradeManager::~TradeManager() {}

void TradeManager::_reset() {
    m_last_update_datetime = m_init_datetime;
    m_cash = m_init_cash;
    m_checkin_cash = m_init_cash;
    m_checkout_cash = 0.0;
    m_checkin_stock = 0.0;
    m_checkout_stock = 0.0;
    m_borrow_cash = 0.0;

    m_loan_list.clear();
    m_borrow_stock.clear();

    m_trade_list.clear();
    m_trade_list.push_back(TradeRecord(Null<Stock>(), m_init_datetime, BUSINESS_INIT, m_init_cash,
                                       m_init_cash, 0.0, 0, CostRecord(), 0.0, m_cash,
                                       PART_INVALID));

    m_position.clear();
    m_position_history.clear();
    m_actions.clear();
    _saveAction(m_trade_list.back());
}

TradeManagerPtr TradeManager::_clone() {
    auto p = make_shared<TradeManager>(m_init_datetime, m_init_cash, m_costfunc, m_name);
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_init_datetime = m_init_datetime;
    p->m_init_cash = m_init_cash;
    p->m_last_update_datetime = m_last_update_datetime;

    // costfunc是一个公共的函数对象，是共享实现，无须deepcopy
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

    return p;
}

double TradeManager::getMarginRate(const Datetime& datetime, const Stock& stock) {
    // TODO 获取保证金比率，默认固定取60%
    return 0.6;
}

Datetime TradeManager::firstDatetime() const {
    Datetime result;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        if (iter->business == BUSINESS_BUY) {
            result = iter->datetime;
            break;
        }
    }
    return result;
}

double TradeManager::getHoldNumber(const Datetime& datetime, const Stock& stock) {
    // 日期小于账户建立日期，返回0
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // 根据权息信息调整持仓数量
    updateWithWeight(datetime);

    // 如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            return pos_iter->second.number;
        }
        return 0.0;
    }

    // 在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    double number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        // 交易记录中的交易日期已经大于查询日期，则跳出循环
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
                BUSINESS_CHECKIN_STOCK == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business ||
                       BUSINESS_CHECKOUT_STOCK == iter->business) {
                number -= iter->number;

            } else {
                // 其他情况忽略
            }
        }
    }
    return number;
}

double TradeManager::getShortHoldNumber(const Datetime& datetime, const Stock& stock) {
    // 日期小于账户建立日期，返回0
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // 根据权息信息调整持仓数量
    updateWithWeight(datetime);

    // 如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_short_position.find(stock.id());
        if (pos_iter != m_short_position.end()) {
            return pos_iter->second.number;
        }
        return 0;
    }

    // 在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    double number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        // 交易记录中的交易日期已经大于查询日期，则跳出循环
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_SELL_SHORT == iter->business) {
                number += iter->number;

            } else if (BUSINESS_BUY_SHORT == iter->business) {
                number -= iter->number;

            } else {
                // 其他情况忽略
            }
        }
    }
    return number;
}

double TradeManager::getDebtNumber(const Datetime& datetime, const Stock& stock) {
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // 根据权息信息调整持仓数量
    updateWithWeight(datetime);

    if (datetime >= lastDatetime()) {
        borrow_stock_map_type::const_iterator bor_iter;
        bor_iter = m_borrow_stock.find(stock.id());
        if (bor_iter != m_borrow_stock.end()) {
            return bor_iter->second.number;
        }
        return 0;
    }

    double debt_n = 0;
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
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // 根据权息信息调整持仓数量
    updateWithWeight(datetime);

    HKU_IF_RETURN(datetime >= lastDatetime(), m_borrow_cash);

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

TradeRecordList TradeManager::getTradeList(const Datetime& start_date,
                                           const Datetime& end_date) const {
    TradeRecordList result;
    HKU_IF_RETURN(start_date >= end_date, result);

    size_t total = m_trade_list.size();
    HKU_IF_RETURN(total == 0, result);

    TradeRecord temp_record;
    temp_record.datetime = start_date;
    auto low = lower_bound(
      m_trade_list.begin(), m_trade_list.end(), temp_record,
      std::bind(std::less<Datetime>(), std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

    temp_record.datetime = end_date;
    auto high = lower_bound(
      m_trade_list.begin(), m_trade_list.end(), temp_record,
      std::bind(std::less<Datetime>(), std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

    result.insert(result.end(), low, high);

    return result;
}

PositionRecordList TradeManager::getPositionList() const {
    PositionRecordList result;
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); ++iter) {
        result.push_back(iter->second);
    }
    return result;
}

PositionRecordList TradeManager::getShortPositionList() const {
    PositionRecordList result;
    position_map_type::const_iterator iter = m_short_position.begin();
    for (; iter != m_short_position.end(); ++iter) {
        result.push_back(iter->second);
    }
    return result;
}

PositionRecord TradeManager::getPosition(const Datetime& datetime, const Stock& stock) {
    PositionRecord result;
    HKU_IF_RETURN(stock.isNull(), result);
    HKU_IF_RETURN(datetime < m_init_datetime, result);

    // 根据权息信息调整持仓数量
    updateWithWeight(datetime);

    // 如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            result = pos_iter->second;
        }
        return result;
    }

    // 在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    double number = 0.0;
    for (auto iter = m_trade_list.begin(); iter != m_trade_list.end(); ++iter) {
        // 交易记录中的交易日期已经大于查询日期，则跳出循环
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
                BUSINESS_CHECKIN_STOCK == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business ||
                       BUSINESS_CHECKOUT_STOCK == iter->business) {
                number -= iter->number;

            } else {
                // 其他情况忽略
            }
        }
    }

    HKU_IF_RETURN(0.0 == number, result);

    // 倒序遍历历史持仓，寻找最后一条持仓记录
    for (auto iter = m_position_history.rbegin(); iter != m_position_history.rend(); ++iter) {
        if (iter->stock == stock) {
            result = *iter;
            break;
        }
    }

    HKU_WARN_IF(result.stock != stock, "Not found in the history positions, maybe exists error! {}",
                stock);
    result.number = number;
    return result;
}

PositionRecord TradeManager::getShortPosition(const Stock& stock) const {
    HKU_IF_RETURN(stock.isNull(), PositionRecord());
    position_map_type::const_iterator iter;
    iter = m_short_position.find(stock.id());
    return iter == m_short_position.end() ? PositionRecord() : iter->second;
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
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.3f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(cash, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_CHECKIN, in_cash, in_cash,
                                       0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::checkout(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(cash, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false,
                        "{} cash({:<.4f}) must be <= current cash({:<.4f})!", datetime, cash,
                        m_cash);

    m_cash = roundEx(m_cash - out_cash, precision);
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_CHECKOUT, out_cash,
                                       out_cash, 0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                                double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkin Null stock!", datetime);
    HKU_ERROR_IF_RETURN(number == 0, false, "{} {} number is zero!", datetime, stock.market_code());
    HKU_ERROR_IF_RETURN(price <= 0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    // 加入当前持仓
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        m_position[stock.id()] = PositionRecord(stock, datetime, Null<Datetime>(), number, 0.0, 0.0,
                                                number, market_value, 0.0, 0.0, 0.0);
    } else {
        PositionRecord& pos = pos_iter->second;
        pos.number += number;
        // pos.stoploss 不变
        pos.totalNumber += number;
        pos.buyMoney = roundEx(pos.buyMoney + market_value, precision);
        // pos.totalCost 不变
        // pos.totalRisk 不变
        // pos.sellMoney 不变
    }

    // 加入交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_CHECKIN_STOCK, price, price, 0.0,
                                       number, CostRecord(), 0.0, m_cash, PART_INVALID));

    // 更新累计存入资产价值记录
    m_checkin_stock = roundEx(m_checkin_stock + market_value, precision);

    return true;
}

bool TradeManager::checkoutStock(const Datetime& datetime, const Stock& stock, price_t price,
                                 double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkout Null stock!", datetime);
    HKU_ERROR_IF_RETURN(number == 0, false, "{} {} checkout number is zero!", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} checkout price({:<.4f}) must be > 0.0! ",
                        datetime, stock.market_code(), price);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    // 当前是否有持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "Try to checkout nonexistent stock!");

    PositionRecord& pos = pos_iter->second;
    // 取出数量超出了当前持仓数量
    HKU_ERROR_IF_RETURN(number > pos.number, false,
                        "{} {} Try to checkout number({}) beyond position number({})!", datetime,
                        stock.market_code(), number, pos.number);

    int precision = getParam<int>("precision");
    pos.number -= number;
    pos.sellMoney = roundEx(pos.sellMoney + price * number * stock.unit(), precision);

    // 取出后当前所有持仓数量为0，清除当前持仓，存入历史持仓
    if (0 == pos.number) {
        m_position_history.push_back(pos);
        m_position.erase(stock.id());
    }

    // 更新交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_CHECKOUT_STOCK, price, price, 0.0,
                                       number, CostRecord(), 0.0, m_cash, PART_INVALID));

    // 更新累计取出股票价值
    m_checkout_stock = roundEx(m_checkout_stock - price * number * stock.unit(), precision);

    return true;
}

bool TradeManager::borrowCash(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0!", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(cash, precision);
    CostRecord cost = getBorrowCashCost(datetime, cash);
    m_cash = roundEx(m_cash + in_cash - cost.total, precision);
    m_borrow_cash = roundEx(m_borrow_cash + in_cash, precision);
    m_loan_list.push_back(LoanRecord(datetime, in_cash));
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_BORROW_CASH, in_cash,
                                       in_cash, 0.0, 0, cost, 0.0, m_cash, PART_INVALID));
    return true;
}

bool TradeManager::returnCash(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());
    HKU_ERROR_IF_RETURN(m_loan_list.empty(), false, "{} not borrow any cash!", datetime);
    HKU_ERROR_IF_RETURN(datetime < m_loan_list.back().datetime, false,
                        "{} must be >= the datetime({}) of last loan record!", datetime,
                        m_loan_list.back().datetime);

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");

    CostRecord cost, cur_cost;
    price_t in_cash = roundEx(cash, precision);
    price_t return_cash = in_cash;
    list<LoanRecord>::iterator iter = m_loan_list.begin();
    for (; iter != m_loan_list.end(); ++iter) {
        if (return_cash <= iter->value) {
            cur_cost = getReturnCashCost(iter->datetime, datetime, return_cash);
            return_cash = 0.0;
        } else {  // return_cash > loan.value
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

    // 欲归还的钱多余实际欠款
    HKU_ERROR_IF_RETURN(return_cash != 0.0, false, "{} return cash must <= borrowed cash!",
                        datetime);

    price_t out_cash = roundEx(in_cash + cost.total, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false,
                        "{} cash({:<.4f}) must be <= current cash({:<.4f})!", datetime, cash,
                        m_cash);

    return_cash = in_cash;
    do {
        iter = m_loan_list.begin();
        if (return_cash == iter->value) {
            m_loan_list.pop_front();
            break;
        } else if (return_cash < iter->value) {
            iter->value = roundEx(iter->value - return_cash, precision);
            break;
        } else {  // return_cash > iter->value
            return_cash = roundEx(return_cash - iter->value, precision);
            m_loan_list.pop_front();
        }
    } while (!m_loan_list.empty());

    m_cash = roundEx(m_cash - out_cash, precision);
    m_borrow_cash = roundEx(m_borrow_cash - in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_RETURN_CASH, in_cash,
                                       in_cash, 0.0, 0, cost, 0.0, m_cash, PART_INVALID));
    return true;
}

bool TradeManager::borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                               double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkin Null stock!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0, false, "{} {} Try to borrow number is zero!", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    // 加入当前持仓
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    CostRecord cost = getBorrowStockCost(datetime, stock, price, number);

    // 更新现金，扣除借入时花费的成本
    m_cash = roundEx(m_cash - cost.total, precision);

    // 加入交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_BORROW_STOCK, price, price, 0.0,
                                       number, cost, 0.0, m_cash, PART_INVALID));

    // 更新当前借入股票信息
    borrow_stock_map_type::iterator iter = m_borrow_stock.find(stock.id());
    if (iter == m_borrow_stock.end()) {
        BorrowRecord record(stock, number, market_value);
        BorrowRecord::Data data(datetime, price, number);
        record.record_list.push_back(data);
        m_borrow_stock[stock.id()] = record;
    } else {
        // iter->second.stock = stock;
        iter->second.number += number;
        iter->second.value = roundEx(iter->second.value + market_value, precision);
        BorrowRecord::Data data(datetime, price, number);
        iter->second.record_list.push_back(data);
    }

    return true;
}

bool TradeManager::returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                               double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkout Null stock!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0, false, "{} {} return stock number is zero!", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    // 查询借入股票信息
    borrow_stock_map_type::iterator bor_iter = m_borrow_stock.find(stock.id());

    // 并未借入股票
    HKU_ERROR_IF_RETURN(bor_iter == m_borrow_stock.end(), false,
                        "{} {} Try to return nonborrowed stock! ", datetime, stock.market_code());

    BorrowRecord& bor = bor_iter->second;

    // 欲归还的数量大于借入的数量
    HKU_ERROR_IF_RETURN(number > bor.number, false,
                        "{} {} Try to return number({}) > borrow number({})!", datetime,
                        stock.market_code(), number, bor.number);

    // 更新借入股票信息
    int precision = getParam<int>("precision");
    CostRecord cost, cur_cost;
    price_t market_value = 0.0;
    double remain_num = number;
    list<BorrowRecord::Data>::iterator iter = bor.record_list.begin();
    for (; iter != bor.record_list.end(); ++iter) {
        if (remain_num <= iter->number) {
            cur_cost = getReturnStockCost(iter->datetime, datetime, stock, price, number);
            market_value =
              roundEx(market_value + iter->price * remain_num * stock.unit(), precision);
            remain_num = 0;
        } else {  // number > iter->number
            cur_cost = getReturnStockCost(iter->datetime, datetime, stock, price, number);
            market_value =
              roundEx(market_value + iter->price * iter->number * stock.unit(), precision);
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
        } else {  // remain_num > iter->number
            remain_num -= iter->number;
            bor.record_list.pop_front();
        }
    } while (!bor.record_list.empty());

    if (bor.record_list.empty()) {
        m_borrow_stock.erase(bor_iter);
    }

    // 更新现金，扣除归还时花费的成本
    m_cash = roundEx(m_cash - cost.total, precision);

    // 更新交易记录
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_RETURN_STOCK, price, price, 0.0,
                                       number, cost, 0.0, m_cash, PART_INVALID));

    return true;
}

TradeRecord TradeManager::buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                              double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                              SystemPart from) {
    TradeRecord result;
    result.business = BUSINESS_INVALID;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0.0, result, "{} {} numer is zero!", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Buy number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                        "{} {} Buy number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

#if 0  // 取消此处的检查，放松限制，另外也可以提高效率。另外，TM只负责交易管理，不许检查
    //检查当日是否存在日线数据，不存在则认为不可交易
    bd::date daydate = datetime.date();
    KRecord krecord = stock.getKRecord(daydate, KQuery::DAY);
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

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

    // 实际交易需要的现金＝交易数量＊实际交易价格＋交易总成本
    int precision = getParam<int>("precision");
    // price_t money = roundEx(realPrice * number * stock.unit() + cost.total, precision);
    price_t money = roundEx(realPrice * number * stock.unit(), precision);

    if (getParam<bool>("support_borrow_cash")) {
        // 获取要求的本金额
        CostRecord bor_cost = getBorrowCashCost(datetime, money);
        double rate = getMarginRate(datetime, stock);
        price_t x = roundEx(m_cash / rate + cost.total + bor_cost.total, precision);
        if (x < money) {
            // 能够获得的融资不够，自动追加本金
            checkin(datetime, roundUp(money - x, precision));
        }

        // 融资，借入资金
        borrowCash(datetime, roundUp(money, precision));
    }

    HKU_WARN_IF_RETURN(m_cash < roundEx(money + cost.total, precision), result,
                       "{} {} Can't buy, need cash({:<.4f}) > current cash({:<.4f})!", datetime,
                       stock.market_code(), roundEx(money + cost.total, precision), m_cash);

    // 更新现金
    m_cash = roundEx(m_cash - money - cost.total, precision);

    // 加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice, goalPrice, number,
                         cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    // 更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        m_position[stock.id()] = PositionRecord(
          stock, datetime, Null<Datetime>(), number, stoploss, goalPrice, number, money, cost.total,
          roundEx((realPrice - stoploss) * number * stock.unit(), precision), 0.0);
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += number;
        position.stoploss = stoploss;
        position.goalPrice = goalPrice;
        position.totalNumber += number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.totalCost = roundEx(cost.total + position.totalCost, precision);
        position.totalRisk =
          roundEx(position.totalRisk + (realPrice - stoploss) * number * stock.unit(), precision);
    }

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        Datetime realtime, nulltime;
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            realtime =
              (*broker_iter)->buy(datetime, stock.market(), stock.code(), realPrice, number);
            if (realtime != nulltime && realtime > m_broker_last_datetime) {
                m_broker_last_datetime = realtime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::sell(const Datetime& datetime, const Stock& stock, price_t realPrice,
                               double number, price_t stoploss, price_t goalPrice,
                               price_t planPrice, SystemPart from) {
    HKU_CHECK(!std::isnan(number), "sell number should be a valid double!");
    TradeRecord result;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0.0, result, "{} {} number is zero!", datetime,
                        stock.market_code());

    // 对于分红扩股造成不满足最小交易量整数倍的情况，只能通过number=MAX_DOUBLE的方式全仓卖出
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Sell number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
                        "{} {} Sell number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

    // 未持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_TRACE_IF_RETURN(pos_iter == m_position.end(), result,
                        "{} {} This stock was not bought never! ({}, {:<.4f}, {}, {})", datetime,
                        stock.market_code(), datetime, realPrice, number, getSystemPartName(from));

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    PositionRecord& position = pos_iter->second;

    // 调整欲卖出的数量，如果卖出数量等于MAX_DOUBLE，则表示卖出全部
    double real_number = number == MAX_DOUBLE ? position.number : number;

    // 欲卖出的数量大于当前持仓的数量
    HKU_ERROR_IF_RETURN(position.number < real_number, result,
                        "{} {} Try to sell number({}) > number of position({})!", datetime,
                        stock.market_code(), real_number, position.number);

    CostRecord cost = getSellCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    // 更新现金余额
    m_cash = roundEx(m_cash + money - cost.total, precision);

    // 更新交易记录
    result = TradeRecord(stock, datetime, BUSINESS_SELL, planPrice, realPrice, goalPrice,
                         real_number, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    // 更新当前持仓情况
    position.number -= real_number;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    // position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_position_history.push_back(position);
        // 删除当前持仓
        m_position.erase(stock.id());
    }

    // 如果存在借款，则归还
    if (getParam<bool>("support_borrow_cash") && m_borrow_cash > 0.0 && m_cash > 0.0) {
        returnCash(datetime, m_borrow_cash < m_cash ? m_borrow_cash : m_cash);
    }

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        Datetime realtime, nulltime;
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            realtime =
              (*broker_iter)->sell(datetime, stock.market(), stock.code(), realPrice, real_number);
            if (realtime != nulltime && realtime > m_broker_last_datetime) {
                m_broker_last_datetime = realtime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                    double number, price_t stoploss, price_t goalPrice,
                                    price_t planPrice, SystemPart from) {
    TradeRecord result;
    result.business = BUSINESS_INVALID;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0, result, "{} {} numer is zero! ", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Buy number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                        "{} {} Buy number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());
    HKU_ERROR_IF_RETURN(
      stoploss != 0.0 && stoploss < realPrice, result,
      "{} {} Sell short's stoploss({:<.4f}) must be > realPrice({:<.4f}) or = 0! ", datetime,
      stock.market_code(), stoploss, realPrice);

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

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

    // 判断是否存在已借入的股票及其数量
    borrow_stock_map_type::const_iterator bor_iter;
    bor_iter = m_borrow_stock.find(stock.id());
    HKU_ERROR_IF_RETURN(bor_iter == m_borrow_stock.end(), result,
                        "{} {} Non borrowed, can't sell short! ", datetime, stock.market_code());

    double total_borrow_num = bor_iter->second.number;
    double can_sell_num = 0;
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    if (pos_iter == m_short_position.end()) {
        // 借入的股票并未卖出过
        can_sell_num = total_borrow_num;

    } else {
        // 借入的股票已经卖出过
        HKU_ERROR_IF_RETURN(pos_iter->second.number >= total_borrow_num, result,
                            "{} {} Borrowed Stock had all selled!", datetime, stock.market_code());

        // 可以卖出的数量 = 借入的总数 - 已卖出的数量
        can_sell_num = total_borrow_num - pos_iter->second.number;
    }

    // 如果计划卖出的数量大于可卖出的数量，则已可每卖出的数量卖出
    double sell_num = number;
    if (number > can_sell_num) {
        sell_num = can_sell_num;
    }

    CostRecord cost = getSellCost(datetime, stock, realPrice, sell_num);

    price_t money = roundEx(realPrice * sell_num * stock.unit() - cost.total, precision);

    // 更新现金
    m_cash = roundEx(m_cash + money, precision);

    // 加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_SELL_SHORT, planPrice, realPrice, goalPrice,
                         sell_num, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    // 更新当前空头持仓记录
    price_t risk = roundEx((stoploss - realPrice) * sell_num * stock.unit(), precision);

    if (pos_iter == m_short_position.end()) {
        m_short_position[stock.id()] =
          PositionRecord(stock, datetime, Null<Datetime>(), sell_num, stoploss, goalPrice, sell_num,
                         cost.total, cost.total, risk, money);
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

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        Datetime realtime, nulltime;
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            realtime =
              (*broker_iter)->sell(datetime, stock.market(), stock.code(), realPrice, number);
            if (realtime != nulltime && realtime > m_broker_last_datetime) {
                m_broker_last_datetime = realtime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                   double number, price_t stoploss, price_t goalPrice,
                                   price_t planPrice, SystemPart from) {
    TradeRecord result;
    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(number == 0, result, "{} {} number is zero!", datetime,
                        stock.market_code());
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} buyShort number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
                        "{} {} buyShort number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

    // 未持有空头仓位
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    HKU_WARN_IF_RETURN(pos_iter == m_short_position.end(), result,
                       "{} {} This stock was not sell never! ", datetime, stock.market_code());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    PositionRecord& position = pos_iter->second;

    // 调整欲买入的数量，如果买入数量等于MAX_DOUBLE或者大于实际仓位，则表示全部买入
    double real_number =
      (number == MAX_DOUBLE || number > position.number) ? position.number : number;

    CostRecord cost = getBuyCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    // 更新现金余额
    m_cash = roundEx(m_cash - money - cost.total, precision);

    // 更新交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY_SHORT, planPrice, realPrice, goalPrice,
                         real_number, cost, stoploss, m_cash, from);
    m_trade_list.push_back(result);

    // 更新当前空头持仓情况
    position.number -= real_number;
    position.buyMoney = roundEx(position.buyMoney + money + cost.total, precision);
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    // position.sellMoney = roundEx(position.sellMoney, precision);

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_short_position_history.push_back(position);
        // 删除当前持仓
        m_short_position.erase(stock.id());
    }

    if (result.datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        Datetime realtime, nulltime;
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            realtime =
              (*broker_iter)->buy(datetime, stock.market(), stock.code(), realPrice, number);
            if (realtime != nulltime && realtime > m_broker_last_datetime) {
                m_broker_last_datetime = realtime;
            }
        }
    }

    if (getParam<bool>("support_borrow_stock")) {
        returnStock(datetime, stock, realPrice, real_number);
    }

    _saveAction(result);

    return result;
}

price_t TradeManager::cash(const Datetime& datetime, KQuery::KType ktype) {
    // 如果指定时间大于更新权息最后时间，则先更新权息
    if (datetime > m_last_update_datetime) {
        updateWithWeight(datetime);
        return m_cash;
    }

    // 如果指定时间等于最后权息更新时间，则直接返回当前现金
    HKU_IF_RETURN(datetime == m_last_update_datetime, m_cash);

    // 指定时间小于最后权息更新时间，则通过计算指定时刻的资产获取资金余额
    FundsRecord funds = getFunds(datetime, ktype);
    return funds.cash;
}

FundsRecord TradeManager::getFunds(KQuery::KType inktype) const {
    FundsRecord funds;
    int precision = getParam<int>("precision");

    string ktype(inktype);
    to_upper(ktype);

    price_t value{0.0};  // 当前市值
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        auto price = record.stock.getMarketValue(lastDatetime(), ktype);
        value = roundEx((value + record.number * price * record.stock.unit()), precision);
    }

    price_t short_value = 0.0;  // 当前空头仓位市值
    iter = m_short_position.begin();
    for (; iter != m_short_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        auto price = record.stock.getMarketValue(lastDatetime(), ktype);
        short_value =
          roundEx((short_value + record.number * price * record.stock.unit()), precision);
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

FundsRecord TradeManager::getFunds(const Datetime& indatetime, KQuery::KType ktype) {
    FundsRecord funds;
    int precision = getParam<int>("precision");

    // // datetime为Null时，直接返回当前账户中的现金和买入时占用的资金，以及累计存取资金
    // HKU_IF_RETURN(indatetime == Null<Datetime>() || indatetime == lastDatetime(),
    // getFunds(ktype));

    Datetime datetime(indatetime.year(), indatetime.month(), indatetime.day(), 23, 59);
    price_t market_value = 0.0;
    price_t short_market_value = 0.0;
    if (datetime > lastDatetime()) {
        // 根据权息数据调整持仓
        updateWithWeight(datetime);

        // 查询日期大于等于最后交易日期时，直接计算当前持仓证券的市值
        position_map_type::const_iterator iter = m_position.begin();
        for (; iter != m_position.end(); ++iter) {
            price_t price = iter->second.stock.getMarketValue(datetime, ktype);
            market_value = roundEx(
              market_value + price * iter->second.number * iter->second.stock.unit(), precision);
        }

        iter = m_short_position.begin();
        for (; iter != m_short_position.end(); ++iter) {
            price_t price = iter->second.stock.getMarketValue(datetime, ktype);
            short_market_value =
              roundEx(short_market_value + price * iter->second.number * iter->second.stock.unit(),
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
    }  // if datetime >= lastDatetime()

    // 当查询日期小于最后交易日期时，遍历交易记录，计算当日的市值和现金
    price_t cash = m_init_cash;
    struct Stock_Number {
        Stock_Number() : number(0) {}
        Stock_Number(const Stock& stock, size_t number) : stock(stock), number(number) {}

        Stock stock;
        size_t number;
    };

    price_t checkin_cash = 0.0;
    price_t checkout_cash = 0.0;
    price_t checkin_stock = 0.0;
    price_t checkout_stock = 0.0;
    map<uint64_t, Stock_Number> stock_map;
    map<uint64_t, Stock_Number> short_stock_map;
    map<uint64_t, Stock_Number>::iterator stock_iter;
    map<uint64_t, Stock_Number>::iterator short_stock_iter;
    map<uint64_t, BorrowRecord> bor_stock_map;
    map<uint64_t, BorrowRecord>::iterator bor_stock_iter;

    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        if (iter->datetime > datetime) {
            // 如果交易记录的日期大于指定的日期则跳出循环，处理完毕
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
                    stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
                }
                break;

            case BUSINESS_SELL:
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter != stock_map.end()) {
                    stock_iter->second.number -= iter->number;
                } else {
                    HKU_WARN("{} {} Sell error in m_trade_list!", datetime,
                             iter->stock.market_code());
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
                    HKU_WARN("{} {} BuyShort Error in m_trade_list!", datetime,
                             iter->stock.market_code());
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
                    stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
                }
                checkin_stock = roundEx(
                  checkin_stock + iter->realPrice * iter->number * iter->stock.unit(), precision);
                break;

            case BUSINESS_CHECKOUT_STOCK:
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter != stock_map.end()) {
                    stock_map[iter->stock.id()].number -= iter->number;
                } else {
                    HKU_WARN("{} {} CheckoutStock Error in m_trade_list!", datetime,
                             iter->stock.market_code());
                }
                checkout_stock = roundEx(
                  checkout_stock + iter->realPrice * iter->number * iter->stock.unit(), precision);
                break;

            case BUSINESS_BORROW_CASH:
                funds.borrow_cash += iter->realPrice;
                break;

            case BUSINESS_RETURN_CASH:
                funds.borrow_cash -= iter->realPrice;
                break;

            case BUSINESS_BORROW_STOCK:
                funds.borrow_asset =
                  roundEx(funds.borrow_asset + iter->realPrice * iter->number * iter->stock.unit(),
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
                    HKU_WARN("{} {} Error return stock in m_trade_list!", iter->datetime,
                             iter->stock.market_code());

                } else {
                    BorrowRecord& bor = bor_stock_iter->second;
                    size_t remain_num = iter->number;
                    do {
                        list<BorrowRecord::Data>::iterator bor_iter = bor.record_list.begin();
                        if (remain_num == bor_iter->number) {
                            funds.borrow_asset -=
                              roundEx(bor_iter->price * remain_num * iter->stock.unit(), precision);
                            bor.record_list.pop_front();
                            break;

                        } else if (remain_num < bor_iter->number) {
                            funds.borrow_asset -=
                              roundEx(bor_iter->price * remain_num * iter->stock.unit(), precision);
                            bor_iter->number -= remain_num;
                            break;

                        } else {  // remain_num > bor_iter->number
                            funds.borrow_asset -= roundEx(
                              bor_iter->price * bor_iter->number * iter->stock.unit(), precision);
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
                HKU_WARN("{} {} Unknown business in m_trade_list!", datetime,
                         iter->stock.market_code());
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
        market_value =
          roundEx(market_value + price * number * stock_iter->second.stock.unit(), precision);
    }

    short_stock_iter = short_stock_map.begin();
    for (; short_stock_iter != short_stock_map.end(); ++short_stock_iter) {
        const size_t& number = short_stock_iter->second.number;
        if (number == 0) {
            continue;
        }

        price_t price = short_stock_iter->second.stock.getMarketValue(datetime, ktype);
        short_market_value =
          roundEx(short_market_value + price * number * stock_iter->second.stock.unit(), precision);
    }

    funds.cash = cash;
    funds.market_value = market_value;
    funds.short_market_value = short_market_value;
    funds.base_cash = checkin_cash - checkout_cash;
    funds.base_asset = checkin_stock - checkout_stock;
    return funds;
}

/******************************************************************************
 *  每次执行交易操作时，先根据权息信息调整持有仓位及现金记录
 *  采用滞后更新的策略，即只在需要获取当前持仓情况及卖出时更新当前的持仓及资产情况
 *  输入参数： 本次操作的日期
 *  历史记录： 1) 2009/12/22 added
 *****************************************************************************/
void TradeManager::updateWithWeight(const Datetime& datetime) {
    HKU_IF_RETURN(datetime <= m_last_update_datetime, void());

    // 权息信息查询日期范围
    Datetime start_date(lastDatetime().date() + bd::days(1));
    Datetime end_date(datetime.date() + bd::days(1));

    int precision = getParam<int>("precision");
    TradeRecordList new_trade_buffer;

    // 更新持仓信息，并缓存新增的交易记录
    position_map_type::iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        PositionRecord& position = position_iter->second;
        Stock stock = position.stock;

        StockWeightList weights = stock.getWeight(start_date, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for (; weight_iter != weights.end(); ++weight_iter) {
            // 如果没有红利并且也（派股和转增股数量都为零），则跳过
            if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
                0.0 == weight_iter->increasement()) {
                continue;
            }

            // 必须在加入配送股之前，因为配送股会引起持仓数量的变化
            if (weight_iter->bonus() != 0.0) {
                price_t bonus = roundEx(position.number * weight_iter->bonus() * 0.1, precision);
                position.sellMoney += bonus;
                m_cash += bonus;

                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_BONUS, bonus, bonus,
                                   0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            price_t addcount =
              (position.number / 10.0) * (weight_iter->countAsGift() + weight_iter->increasement());
            if (addcount != 0.0) {
                position.number += addcount;
                position.totalNumber += addcount;
                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_GIFT, 0.0, 0.0, 0.0,
                                   addcount, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }
        } /* for weight */
    } /* for position */

    std::sort(
      new_trade_buffer.begin(), new_trade_buffer.end(),
      std::bind(std::less<Datetime>(), std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

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

    m_last_update_datetime = datetime;
}

void TradeManager::_saveAction(const TradeRecord& record) {
    HKU_IF_RETURN(getParam<bool>("save_action") == false, void());
    std::stringstream buf(std::stringstream::out);
    string my_tm("td = my_tm.");
    string sep(", ");
    switch (record.business) {
        case BUSINESS_INIT:
            buf << "my_tm = crtTM(datetime=Datetime('" << record.datetime.str() << "'), "
                << "initCash=" << record.cash << sep << "costFunc=" << m_costfunc->name() << "("
                << m_costfunc->getParameter().getNameValueList() << "), " << "name='" << m_name
                << "'" << ")";
            break;

        case BUSINESS_CHECKIN:
            buf << my_tm << "checkin(Datetime('" << record.datetime.str() << "'), " << record.cash
                << ")";
            break;

        case BUSINESS_CHECKOUT:
            buf << my_tm << "checkout(Datetime('" << record.datetime.str() << "'), " << record.cash
                << ")";
            break;

        case BUSINESS_BUY:
            buf << my_tm << "buy(Datetime('" << record.datetime.str() << "'), " << "sm['"
                << record.stock.market_code() << "'], " << record.realPrice << sep << record.number
                << sep << record.stoploss << sep << record.goalPrice << sep << record.planPrice
                << sep << record.from << ")";
            break;

        case BUSINESS_SELL:
            buf << my_tm << "sell(Datetime('" << record.datetime.str() << "')," << "sm['"
                << record.stock.market_code() << "'], " << record.realPrice << sep << record.number
                << sep << record.stoploss << sep << record.goalPrice << sep << record.planPrice
                << sep << record.from << ")";
            break;

        default:
            break;
    }

    m_actions.push_back(buf.str());
}

void TradeManager::tocsv(const string& path) {
    string filename1, filename2, filename3, filename4;
    if (m_name.empty()) {
        string date = m_init_datetime.str();
        filename1 = path + "/" + date + "_交易记录.csv";
        filename2 = path + "/" + date + "_已平仓记录.csv";
        filename3 = path + "/" + date + "_未平仓记录.csv";
        filename4 = path + "/" + date + "_actions.txt";
    } else {
        filename1 = path + "/" + m_name + "_交易记录.csv";
        filename2 = path + "/" + m_name + "_已平仓记录.csv";
        filename3 = path + "/" + m_name + "_未平仓记录.csv";
        filename4 = path + "/" + m_name + "_actions.txt";
    }

#if defined(_MSC_VER)
    filename1 = utf8_to_gb(filename1);
    filename2 = utf8_to_gb(filename2);
    filename3 = utf8_to_gb(filename3);
    filename4 = utf8_to_gb(filename4);
#endif

    string sep(",");

    // 导出交易记录
    std::ofstream file(filename1.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename1);

    file.setf(std::ios_base::fixed);
    file.precision(3);
    file << "#成交日期,证券代码,证券名称,业务名称,计划交易价格,"
            "实际成交价格,目标价格,成交数量,佣金,印花税,过户费,其他成本,交易总成本,"
            "止损价,现金余额,信号来源,日期,开盘价,最高价,最低价,收盘价,"
            "成交金额,成交量"
         << std::endl;
    TradeRecordList::const_iterator trade_iter = m_trade_list.begin();
    for (; trade_iter != m_trade_list.end(); ++trade_iter) {
        const TradeRecord& record = *trade_iter;
        if (record.stock.isNull()) {
            file << record.datetime << sep << sep << sep << getBusinessName(record.business) << sep
                 << record.planPrice << sep << record.realPrice << sep << record.goalPrice << sep
                 << record.number << sep << record.cost.commission << sep << record.cost.stamptax
                 << sep << record.cost.transferfee << sep << record.cost.others << sep
                 << record.cost.total << sep << record.stoploss << sep << record.cash << sep
                 << getSystemPartName(record.from) << std::endl;
        } else {
            file << record.datetime << sep << record.stock.market_code() << sep
                 << record.stock.name() << sep << getBusinessName(record.business) << sep
                 << record.planPrice << sep << record.realPrice << sep << record.goalPrice << sep
                 << record.number << sep << record.cost.commission << sep << record.cost.stamptax
                 << sep << record.cost.transferfee << sep << record.cost.others << sep
                 << record.cost.total << sep << record.stoploss << sep << record.cash << sep
                 << getSystemPartName(record.from) << sep;
            if (BUSINESS_BUY == record.business || BUSINESS_SELL == record.business) {
                KRecord kdata = record.stock.getKRecord(record.datetime, KQuery::DAY);
                if (kdata.isValid()) {
                    file << kdata.datetime << sep << kdata.openPrice << sep << kdata.highPrice
                         << sep << kdata.lowPrice << sep << kdata.closePrice << sep
                         << kdata.transAmount << sep << kdata.transCount;
                }
            }
            file << std::endl;
        }
    }
    file.close();

    // 导出已平仓记录
    file.open(filename2.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename2);
    file << "#建仓日期,平仓日期,证券代码,证券名称,累计持仓数量,"
            "累计花费资金,累计交易成本,已转化资金,总盈利,累积风险"
         << std::endl;
    PositionRecordList::const_iterator history_iter = m_position_history.begin();
    for (; history_iter != m_position_history.end(); ++history_iter) {
        const PositionRecord& record = *history_iter;
        file << record.takeDatetime << sep << record.cleanDatetime << sep
             << record.stock.market_code() << sep << record.stock.name() << sep
             << record.totalNumber << sep << record.buyMoney << sep << record.totalCost << sep
             << record.sellMoney << sep << record.sellMoney - record.totalCost - record.buyMoney
             << sep << record.totalRisk << std::endl;
    }
    file.close();

    // 导出未平仓记录
    file.open(filename3.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename3);
    file << "#建仓日期,平仓日期,证券代码,证券名称,当前持仓数量,累计持仓数量,"
            "累计花费资金,累计交易成本,已转化资金,累积风险,"
            "累计浮动盈亏,当前盈亏成本价"
         << std::endl;
    position_map_type::const_iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        const PositionRecord& record = position_iter->second;
        file << record.takeDatetime << sep << record.cleanDatetime << sep
             << record.stock.market_code() << sep << record.stock.name() << sep << record.number
             << sep << record.totalNumber << sep << record.buyMoney << sep << record.totalCost
             << sep << record.sellMoney << sep << record.totalRisk << sep;
        size_t pos = record.stock.getCount(KQuery::DAY);
        if (pos != 0) {
            KRecord krecord = record.stock.getKRecord(pos - 1, KQuery::DAY);
            price_t bonus = record.buyMoney - record.sellMoney - record.totalCost;
            file << record.number * krecord.closePrice - bonus << sep << bonus / record.number
                 << std::endl;
        }
    }
    file.close();

    // 到处执行命令
    // 导出已平仓记录
    file.open(filename4.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename4);
    list<string>::const_iterator action_iter = m_actions.begin();
    for (; action_iter != m_actions.end(); ++action_iter) {
        file << *action_iter << std::endl;
    }
    file.close();
}

bool TradeManager::addTradeRecord(const TradeRecord& tr) {
    HKU_IF_RETURN(BUSINESS_INIT == tr.business, _add_init_tr(tr));
    HKU_ERROR_IF_RETURN(tr.datetime < lastDatetime(), false,
                        "tr.datetime must be >= lastDatetime({})!", lastDatetime());

    updateWithWeight(tr.datetime);

    switch (tr.business) {
        case BUSINESS_INIT:
            return false;

        case BUSINESS_BUY:
            return _add_buy_tr(tr);

        case BUSINESS_SELL:
            return _add_sell_tr(tr);

        case BUSINESS_GIFT:
            return true;

        case BUSINESS_BONUS:
            return true;

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

        case BUSINESS_INVALID:
        default:
            HKU_ERROR("tr.business is invalid({})!", int(tr.business));
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
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(tr.number == 0, false, "tr.number is zero!");
    HKU_ERROR_IF_RETURN(
      tr.number < tr.stock.minTradeNumber() || tr.number > tr.stock.maxTradeNumber(), false,
      "tr.number out of range!");

    int precision = getParam<int>("precision");
    TradeRecord new_tr(tr);
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    HKU_WARN_IF_RETURN(m_cash < roundEx(money + tr.cost.total, precision), false,
                       "Don't have enough money! {} < {}, {}", m_cash,
                       roundEx(money + tr.cost.total, precision), tr);

    m_cash = roundEx(m_cash - money - tr.cost.total, precision);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    // 更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        m_position[tr.stock.id()] = PositionRecord(
          tr.stock, tr.datetime, Null<Datetime>(), tr.number, tr.stoploss, tr.goalPrice, tr.number,
          money, tr.cost.total,
          roundEx((tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(), precision), 0.0);
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += tr.number;
        position.stoploss = tr.stoploss;
        position.goalPrice = tr.goalPrice;
        position.totalNumber += tr.number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.totalCost = roundEx(tr.cost.total + position.totalCost, precision);
        position.totalRisk =
          roundEx(position.totalRisk + (tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(),
                  precision);
    }

    _saveAction(new_tr);

    return true;
}

bool TradeManager::_add_sell_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is Null!");
    HKU_ERROR_IF_RETURN(tr.number == 0, false, "tr.number is zero!");

    // 未持仓
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "No position!");

    PositionRecord& position = pos_iter->second;

    // 欲卖出的数量大于当前持仓的数量
    HKU_ERROR_IF_RETURN(position.number < tr.number, false, "Try sell number greater position!");

    int precision = getParam<int>("precision");
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    // 更新现金余额
    m_cash = roundEx(m_cash + money - tr.cost.total, precision);

    // 更新交易记录
    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    // 更新当前持仓情况
    position.number -= tr.number;
    position.stoploss = tr.stoploss;
    position.goalPrice = tr.goalPrice;
    // position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + tr.cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);

    if (position.number == 0) {
        position.cleanDatetime = tr.datetime;
        m_position_history.push_back(position);
        // 删除当前持仓
        m_position.erase(tr.stock.id());
    }

    _saveAction(new_tr);

    return true;
}

bool TradeManager::_add_checkin_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.realPrice <= 0.0, false, "tr.realPrice <= 0.0!");
    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(tr.realPrice, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), tr.datetime, BUSINESS_CHECKIN, in_cash,
                                       in_cash, 0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::_add_checkout_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.realPrice <= 0.0, false, "tr.realPrice <= 0.0!");

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(tr.realPrice, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false, "Checkout money > current cash!");

    m_cash = roundEx(m_cash - out_cash, precision);
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), tr.datetime, BUSINESS_CHECKOUT, out_cash,
                                       out_cash, 0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::_add_checkin_stock_tr(const TradeRecord& tr) {
    // TODO: TradeManager::_add_checkin_stock_tr
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
