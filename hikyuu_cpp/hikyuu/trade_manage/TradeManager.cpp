/*
 * TradeManager.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <fstream>
#include <sstream>
#include <cmath>
#include <functional>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "TradeManager.h"
#include "margin/crt/MG_Fixed.h"
#include "../lang.h"
#include "../trade_sys/system/SystemPart.h"
#include "../KData.h"
#include "../utilities/thread/algorithm.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TradeManager)
#endif

namespace hku {

/** The proportional share of the still-held opening notional (money) for the given quantity */
static price_t positionOpenBasis(const PositionRecord& position, double number) {
    HKU_IF_RETURN(position.number <= 0.0, 0.0);
    return position.openBasis * (number / position.number);
}

/** The proportional share of the still-held mark-to-market basis for the given quantity */
static price_t positionSettleBasis(const PositionRecord& position, double number) {
    HKU_IF_RETURN(position.number <= 0.0, 0.0);
    return position.settleBasis * (number / position.number);
}

/**
 * The equity contribution of a margin-accounted position: the locked margin plus the unrealized
 * profit measured against the settlement basis at the given price; the short side contributes as
 * a liability instead
 */
static price_t marginPositionEquity(const PositionRecord& position, price_t price, bool is_long) {
    price_t mv = price * position.number * position.stock.unit();
    price_t basis = positionSettleBasis(position, position.number);
    return is_long ? position.margin + (mv - basis) : position.margin - (mv - basis);
}

string TradeManager::str() const {
    std::stringstream os;
    os << std::fixed;
    (void)os.precision(2);

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
       << "  current borrow_asset: " << funds.borrow_asset << strip
       << "  current occupied_margin: " << funds.occupied_margin << strip << "  Position: \n";

    StockManager& sm = StockManager::instance();
    KQuery query(-1);
    PositionRecordList position = getPositionList();
    PositionRecordList::const_iterator iter = position.begin();
    os << "    "
       << htr(
            "code name takeDatetime hold_days number invest market_value bonus "
            "return_rate "
            "initial_capital_return\n");
    for (; iter != position.end(); ++iter) {
        price_t invest = iter->buyMoney - iter->sellMoney + iter->totalCost;
        KData k = iter->stock.getKData(query);
        price_t cur_val = k.size() > 0 ? k[0].closePrice * iter->number : 0.0;
        price_t bonus = cur_val - invest;
        DatetimeList date_list =
          sm.getTradingCalendar(KQueryByDate(Datetime(iter->takeDatetime.date())));
        os << "    " << iter->stock.market_code() << " " << iter->stock.name() << " "
           << iter->takeDatetime << " " << date_list.size() << " " << iter->number << " " << invest
           << " " << cur_val << " " << bonus << " " << (invest != 0.0 ? 100 * bonus / invest : 0.0)
           << "% " << (m_init_cash != 0.0 ? 100 * bonus / m_init_cash : 0.0) << "%\n";
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
    (void)os.precision();
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
    setParam<bool>("support_borrow_cash", false);   // Whether to finance automatically
    setParam<bool>("support_borrow_stock", false);  // Whether to borrow stocks automatically
    setParam<bool>("save_action", true);            // Whether to save the commands
    m_margin = MG_Fixed(0.6);  // Legacy behaviour: margin financing at 60 % buying power
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
    m_tmp_weight_checked_until.clear();
    m_tmp_margin_called = false;
    m_tmp_force_liquidate_date = Null<Datetime>();
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
    m_short_position.clear();
    m_short_position_history.clear();
    m_actions.clear();
    _saveAction(m_trade_list.back());
}

TradeManagerPtr TradeManager::_clone() {
    auto p = make_shared<TradeManager>(m_init_datetime, m_init_cash, m_costfunc, m_name);
    p->m_init_datetime = m_init_datetime;
    p->m_init_cash = m_init_cash;
    p->m_last_update_datetime = m_last_update_datetime;
    p->m_tmp_weight_checked_until = m_tmp_weight_checked_until;
    p->m_tmp_margin_called = m_tmp_margin_called;
    p->m_tmp_force_liquidate_date = m_tmp_force_liquidate_date;

    p->m_cash = m_cash;
    p->m_margin = m_margin;
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
    p->m_short_position = m_short_position;
    p->m_short_position_history = m_short_position_history;
    p->m_actions = m_actions;
    return p;
}

double TradeManager::getMarginRate(const Datetime& datetime, const Stock& stock) {
    return m_margin ? m_margin->getMarginRate(datetime, stock, MarginSide::LONG) : 1.0;
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
    // The date is earlier than the account creation date, return 0
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // Adjust the position quantity according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    // If the given date is later than or equal to the last trade date, take the current position
    // record directly
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            return pos_iter->second.number;
        }
        return 0.0;
    }

    // In the historical trade records, recalculate the position quantity of the trading object at
    // the given query date
    double number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        // Break the loop when the trade date in the trade record is already later than the query
        // date
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
                BUSINESS_CHECKIN_STOCK == iter->business || BUSINESS_SUOGU == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business ||
                       BUSINESS_CHECKOUT_STOCK == iter->business) {
                number -= iter->number;

            } else {
                // The other cases are ignored
            }
        }
    }
    return number;
}

double TradeManager::getShortHoldNumber(const Datetime& datetime, const Stock& stock) {
    // The date is earlier than the account creation date, return 0
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // Adjust the position quantity according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    // If the given date is later than or equal to the last trade date, take the current position
    // record directly
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_short_position.find(stock.id());
        if (pos_iter != m_short_position.end()) {
            return pos_iter->second.number;
        }
        return 0;
    }

    // In the historical trade records, recalculate the position quantity of the trading object at
    // the given query date
    double number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        // Break the loop when the trade date in the trade record is already later than the query
        // date
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_SELL_SHORT == iter->business) {
                number += iter->number;

            } else if (BUSINESS_BUY_SHORT == iter->business) {
                number -= iter->number;

            } else if (BUSINESS_SHORT_ADJUST == iter->business) {
                number += iter->number;

            } else {
                // The other cases are ignored
            }
        }
    }
    return number;
}

double TradeManager::getDebtNumber(const Datetime& datetime, const Stock& stock) {
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // Adjust the position quantity according to the ex-rights/ex-dividend information
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
            } else if (iter->business == BUSINESS_BORROW_ADJUST) {
                debt_n += iter->number;
            }
        }
    }
    return debt_n;
}

price_t TradeManager::getDebtCash(const Datetime& datetime) {
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    // Adjust the position quantity according to the ex-rights/ex-dividend information
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

    // Adjust the position quantity according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    // If the given date is later than or equal to the last trade date, take the current position
    // record directly
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            result = pos_iter->second;
        }
        return result;
    }

    // In the historical trade records, recalculate the position quantity of the trading object at
    // the given query date
    double number = 0.0;
    for (auto iter = m_trade_list.begin(); iter != m_trade_list.end(); ++iter) {
        // Break the loop when the trade date in the trade record is already later than the query
        // date
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
                BUSINESS_CHECKIN_STOCK == iter->business || BUSINESS_SUOGU == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business ||
                       BUSINESS_CHECKOUT_STOCK == iter->business) {
                number -= iter->number;

            } else {
                // The other cases are ignored
            }
        }
    }

    HKU_IF_RETURN(0.0 == number, result);

    // Traverse the historical positions in reverse order to find the last position record
    for (auto iter = m_position_history.rbegin(); iter != m_position_history.rend(); ++iter) {
        if (iter->stock == stock) {
            result = *iter;
            result.number = number;
            return result;
        }
    }

    // Not closed by the query date: fall back to the currently open position so the returned
    // record carries a real stock and economic fields (not an inconsistent default with a
    // positive number and a null stock)
    position_map_type::const_iterator open = m_position.find(stock.id());
    HKU_ERROR_IF_RETURN(open == m_position.end(), Null<PositionRecord>(),
                        "{} Not found in the history positions, maybe exists error!", stock);
    PositionRecord cur = open->second;
    cur.number = number;
    return cur;
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

    // Adjust the current position according to the ex-rights/ex-dividend information
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

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(cash, precision);

    price_t tmp_cash = roundEx(m_cash - out_cash, precision);
    HKU_ERROR_IF_RETURN(tmp_cash < 0.0, false, "{} cash({:<.4f}) must be <= current cash({:<.4f})!",
                        datetime, cash, m_cash);

    m_cash = tmp_cash;
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.push_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_CHECKOUT, out_cash,
                                       out_cash, 0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID));
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                                double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkin Null stock!", datetime);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number <= 0, false,
                        "{} {} number({}) must be finite and > 0!", datetime, stock.market_code(),
                        number);
    HKU_ERROR_IF_RETURN(price <= 0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);
    // The holding grows at this datetime; drop the check frontier for this stock (see buy)
    m_tmp_weight_checked_until.erase(stock.id());

    // Add it to the current positions
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord pos(stock, datetime, Null<Datetime>(), number, 0.0, 0.0, number,
                           market_value, 0.0, 0.0, 0.0);
        pos.buyCount = 1;
        m_position[stock.id()] = pos;
    } else {
        PositionRecord& pos = pos_iter->second;
        // A deposit is a cash-basis lot: joining a margin-accounted holding would double the basis
        // while the lock stays, so the equity would silently stop matching the assets
        HKU_ERROR_IF_RETURN(pos.margin > 0.0, false,
                            "{} {} Can't deposit into a margin-accounted holding, its margin books "
                            "are not supported yet!",
                            datetime, stock.market_code());
        pos.number += number;
        // pos.stoploss stays unchanged
        pos.totalNumber += number;
        pos.buyMoney = roundEx(pos.buyMoney + market_value, precision);
        pos.openBasis = roundEx(pos.openBasis + market_value, precision);
        pos.settleBasis = roundEx(pos.settleBasis + market_value, precision);
        // pos.totalCost stays unchanged
        // pos.totalRisk stays unchanged
        // pos.sellMoney stays unchanged
        pos.buyCount++;
    }

    // Add it to the trade records
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_CHECKIN_STOCK, price, price, 0.0,
                                       number, CostRecord(), 0.0, m_cash, PART_INVALID));

    // Update the record of the accumulated deposited asset value
    m_checkin_stock = roundEx(m_checkin_stock + market_value, precision);

    return true;
}

bool TradeManager::checkoutStock(const Datetime& datetime, const Stock& stock, price_t price,
                                 double number) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkout Null stock!", datetime);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number <= 0, false,
                        "{} {} checkout number({}) must be finite and > 0!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} checkout price({:<.4f}) must be > 0.0! ",
                        datetime, stock.market_code(), price);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    // Whether there is a current position
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "Try to checkout nonexistent stock!");

    PositionRecord& pos = pos_iter->second;
    // The withdrawn quantity exceeds the current position quantity
    HKU_ERROR_IF_RETURN(number > pos.number, false,
                        "{} {} Try to checkout number({}) beyond position number({})!", datetime,
                        stock.market_code(), number, pos.number);

    int precision = getParam<int>("precision");
    // A withdrawal is a cash-basis transfer that would take the locked margin of the withdrawn lots
    // along with them, so it is rejected on a margin-accounted holding
    HKU_ERROR_IF_RETURN(pos.margin > 0.0, false,
                        "{} {} Can't withdraw from a margin-accounted holding, its margin books "
                        "are not supported yet!",
                        datetime, stock.market_code());
    // Withdrawing lots releases their cost basis proportionally (the price only feeds the
    // reporting sellMoney, the cost pool follows the quantity)
    price_t basis = positionOpenBasis(pos, number);
    price_t mtmBasis = positionSettleBasis(pos, number);
    pos.number -= number;
    pos.openBasis = pos.number > 0.0 ? roundEx(pos.openBasis - basis, precision) : 0.0;
    pos.settleBasis = pos.number > 0.0 ? roundEx(pos.settleBasis - mtmBasis, precision) : 0.0;
    pos.sellMoney = roundEx(pos.sellMoney + price * number * stock.unit(), precision);
    pos.sellCount++;

    // After the withdrawal all the current position quantities become 0, clear the current position
    // and store it into the historical positions
    if (0 == pos.number) {
        pos.cleanDatetime = datetime;
        m_position_history.push_back(pos);
        m_position.erase(stock.id());
    }

    // Update the trade records
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_CHECKOUT_STOCK, price, price, 0.0,
                                       number, CostRecord(), 0.0, m_cash, PART_INVALID));

    // Update the accumulated withdrawn stock value
    m_checkout_stock = roundEx(m_checkout_stock + price * number * stock.unit(), precision);

    return true;
}

bool TradeManager::borrowCash(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0!", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    // Adjust the current position according to the ex-rights/ex-dividend information
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

bool TradeManager::_borrowForAdd(const Datetime& datetime, price_t needed) {
    HKU_IF_RETURN(!getParam<bool>("support_borrow_cash"), false);

    int precision = getParam<int>("precision");
    price_t gap = roundEx(needed - m_cash, precision);
    HKU_IF_RETURN(gap <= 0.0, true);

    // The borrow fee is paid out of the borrowed cash, so the gap grows until it covers that fee as
    // well (the iteration of buy / sellShort, on the add path)
    for (int i = 0; i < 32; ++i) {
        price_t after = roundEx(m_cash + gap - getBorrowCashCost(datetime, gap).total, precision);
        if (after >= needed) {
            return borrowCash(datetime, gap);
        }
        price_t next = roundEx(gap + roundEx(needed - after, precision), precision);
        HKU_WARN_IF_RETURN(next <= gap, false,
                           "{} Can't finance the replayed opening, the borrow cost can not be "
                           "covered by gap({:<.4f})!",
                           datetime, gap);
        gap = next;
    }
    return false;
}

bool TradeManager::returnCash(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());
    HKU_ERROR_IF_RETURN(m_loan_list.empty(), false, "{} not borrow any cash!", datetime);
    HKU_ERROR_IF_RETURN(datetime < m_loan_list.back().datetime, false,
                        "{} must be >= the datetime({}) of last loan record!", datetime,
                        m_loan_list.back().datetime);

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");

    // All the values live on the rounding grid of the precision; treat the sub-grid residual as
    // zero so that the rounding drift cannot fail the equality based termination and validation
    price_t epsilon = 0.5 * std::pow(10.0, -double(precision));

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
        if (return_cash < epsilon)
            break;
    }

    // The money to be returned is more than the actual debt (the residual below the grid unit is
    // treated as fully repaid)
    HKU_ERROR_IF_RETURN(return_cash > epsilon, false, "{} return cash must <= borrowed cash!",
                        datetime);

    price_t out_cash = roundEx(in_cash + cost.total, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false,
                        "{} cash({:<.4f}) must be <= current cash({:<.4f})!", datetime, cash,
                        m_cash);

    return_cash = in_cash;
    do {
        iter = m_loan_list.begin();
        if (std::abs(return_cash - iter->value) < epsilon) {
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
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number <= 0, false,
                        "{} {} borrow number({}) must be finite and > 0!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);
    // The borrow debt grows at this datetime; drop the check frontier for this stock (see buy)
    m_tmp_weight_checked_until.erase(stock.id());

    // Add it to the current positions
    int precision = getParam<int>("precision");
    price_t market_value = roundEx(price * number * stock.unit(), precision);
    CostRecord cost = getBorrowStockCost(datetime, stock, price, number);

    // Update the cash, deducting the cost spent when borrowing
    m_cash = roundEx(m_cash - cost.total, precision);

    // Add it to the trade records
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_BORROW_STOCK, price, price, 0.0,
                                       number, cost, 0.0, m_cash, PART_INVALID));

    // Update the current borrowed stock information
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
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number <= 0, false,
                        "{} {} return number({}) must be finite and > 0!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(price <= 0.0, false, "{} {} price({:<.4f}) must be > 0!", datetime,
                        stock.market_code(), price);

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    // Query the borrowed stock information
    borrow_stock_map_type::iterator bor_iter = m_borrow_stock.find(stock.id());

    // No stock was borrowed
    HKU_ERROR_IF_RETURN(bor_iter == m_borrow_stock.end(), false,
                        "{} {} Try to return nonborrowed stock! ", datetime, stock.market_code());

    BorrowRecord& bor = bor_iter->second;

    // The quantity to be returned is greater than the borrowed quantity
    HKU_ERROR_IF_RETURN(number > bor.number, false,
                        "{} {} Try to return number({}) > borrow number({})!", datetime,
                        stock.market_code(), number, bor.number);

    // Update the borrowed stock information
    int precision = getParam<int>("precision");
    CostRecord cost, cur_cost;
    price_t market_value = 0.0;
    double remain_num = number;
    list<BorrowRecord::Data>::iterator iter = bor.record_list.begin();
    for (; iter != bor.record_list.end(); ++iter) {
        if (remain_num <= iter->number) {
            cur_cost = getReturnStockCost(iter->datetime, datetime, stock, price, remain_num);
            market_value =
              roundEx(market_value + iter->price * remain_num * stock.unit(), precision);
            remain_num = 0;
        } else {  // number > iter->number
            cur_cost = getReturnStockCost(iter->datetime, datetime, stock, price, iter->number);
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

    // Update the cash, deducting the cost spent when returning
    m_cash = roundEx(m_cash - cost.total, precision);

    // Update the trade records
    m_trade_list.push_back(TradeRecord(stock, datetime, BUSINESS_RETURN_STOCK, price, price, 0.0,
                                       number, cost, 0.0, m_cash, PART_INVALID));

    return true;
}

TradeRecord TradeManager::buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                              double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                              SystemPart from, const string& remark) {
    TradeRecord result;
    result.business = BUSINESS_INVALID;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    // realPrice 0 is a legal placeholder for the market-order path of Strategy (the dealing
    // price comes from the broker); only non-finite or negative values are invalid
    HKU_ERROR_IF_RETURN(!std::isfinite(realPrice) || realPrice < 0.0, result,
                        "{} {} Invalid buy real price({})!", datetime, stock.market_code(),
                        realPrice);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number == 0.0, result,
                        "{} {} number({}) must be finite and nonzero!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Buy number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                        "{} {} Buy number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());
    HKU_WARN_IF_RETURN(_isExpired(datetime, stock), result, "{} {} expired({}), can't buy!",
                       datetime, stock.market_code(), stock.lastDatetime());

#if 0  // The check here is cancelled to relax the restriction and improve the efficiency; TM is
       // only responsible for the trade management and is not allowed to check
    // Check whether the daily line data exists for the day; when it does not exist it is regarded as not tradable
    bd::date daydate = datetime.date();
    KRecord krecord = stock.getKRecord(daydate, KQuery::DAY);
    if (!krecord.isValid()) {
        HKU_ERROR(datetime << " " << stock.market_code()
                <<" Non-trading day(" << daydate
                << ") [TradeManager::buy]");
        return result;
    }

    // Whether the buy price is within the highest / lowest price range of the day
    if (realPrice > krecord.highPrice || realPrice < krecord.lowPrice) {
        HKU_ERROR(datetime << " " << stock.market_code()
                << " Invalid buy price(" << realPrice
                << ")! out of highPrice(" << krecord.highPrice
                << ") or lowPrice(" << krecord.lowPrice
                << "! [TradeManager::buy]");
        return result;
    }
#endif

    // The margin gate must run before updateWithWeight: the available margin is read through
    // getFunds, which normalizes the date to 23:59 and applies the ex-rights/ex-dividend weights
    // itself. Reading it after the update below (and after that stock's check frontier is dropped)
    // would rescan the same weight window and book every bonus/gift/suogu twice
    // realPrice 0 is the legal market-order placeholder (the dealing price comes from the
    // broker), the quota cannot be measured without a price and the plain cash/borrow logic
    // below handles it; the gate applies only to priced orders
    // With the margin accounting the cash only shrinks by the margin, so the gate is required for
    // every margin-accounted order (a futures account without support_borrow_cash has none either)
    bool margin_acct = isMarginAccounted(stock) && realPrice > 0.0;
    if ((getParam<bool>("support_borrow_cash") || margin_acct) && realPrice > 0.0) {
        // The admission test uses the margin of the requested order itself instead of the floored
        // whole-unit quota: the latter loses one unit to floating-point rounding and rejects
        // odd-lot orders the account can still afford
        price_t available = getAvailableMargin(datetime);
        price_t need = getOpenMargin(datetime, stock, realPrice, number, MarginSide::LONG);
        double tolerance = std::max(1e-9, available * 1e-12);
        HKU_WARN_IF_RETURN(need > available + tolerance, result,
                           "{} {} Can't buy, needed margin({:<.4f}) > available({:<.4f})!",
                           datetime, stock.market_code(), need, available);
    }

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);
    // The holding grows at this datetime; drop that stock's check frontier so an out-of-order
    // trade earlier than the cached date still rescans (see updateWithWeight incremental cache)
    m_tmp_weight_checked_until.erase(stock.id());

    CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

    // The cash needed by the actual trade = the trade quantity * the actual trade price + the total
    // trade cost
    int precision = getParam<int>("precision");
    // price_t money = roundEx(realPrice * number * stock.unit() + cost.total, precision);
    price_t money = roundEx(realPrice * number * stock.unit(), precision);

    // The cash locked by the opening: only the margin when the holding is booked by margin, the
    // full notional otherwise. The notional itself is still accumulated for the reporting
    // A model that reports no requirement at all can't book the holding by margin: the opening
    // falls back to the cash books instead of counting the full notional as equity for free
    price_t locked_margin =
      margin_acct ? getOpenMarginToLock(datetime, stock, realPrice, number, MarginSide::LONG) : 0.0;
    HKU_WARN_IF(margin_acct && !(locked_margin > 0.0),
                "{} {} The margin model reported no requirement, the opening is booked by cash!",
                datetime, stock.market_code());
    margin_acct = margin_acct && locked_margin > 0.0;
    price_t need = margin_acct ? locked_margin : money;

    if (getParam<bool>("support_borrow_cash")) {
        // Borrow on demand, including the cost of the borrow itself: the buy cost is computed on
        // the real borrow amount (not on the full trade money), and the post-borrow cash is
        // simulated with the same formula borrowCash uses before creating any debt, so a failed
        // buy never leaves a loan record behind
        price_t need_cash = roundEx(need + cost.total, precision);
        price_t gap = roundEx(need_cash - m_cash, precision);
        if (gap > 0.0) {
            bool covered = false;
            for (int i = 0; i < 32 && !covered; ++i) {
                price_t after =
                  roundEx(m_cash + gap - getBorrowCashCost(datetime, gap).total, precision);
                covered = after >= need_cash;
                if (!covered) {
                    price_t next = roundEx(gap + roundEx(need_cash - after, precision), precision);
                    // The cost rate must stay well below 100%; a non-increasing gap means the
                    // sequence stalled and the cost can never be covered
                    HKU_WARN_IF_RETURN(
                      next <= gap, result,
                      "{} {} Can't buy, the borrow cost can not be covered by gap({:<.4f})!",
                      datetime, stock.market_code(), gap);
                    gap = next;
                }
            }
            HKU_WARN_IF_RETURN(
              !covered, result,
              "{} {} Can't buy, the borrow cost can not be covered by gap({:<.4f})!", datetime,
              stock.market_code(), gap);
            borrowCash(datetime, gap);
        }
    }

    HKU_WARN_IF_RETURN(m_cash < roundEx(need + cost.total, precision), result,
                       "{} {} Can't buy, need cash({:<.4f}) > current cash({:<.4f})!", datetime,
                       stock.market_code(), roundEx(need + cost.total, precision), m_cash);

    // Update the cash
    m_cash = roundEx(m_cash - need - cost.total, precision);

    // Add it to the trade records
    result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice, goalPrice, number,
                         cost, stoploss, m_cash, from, remark);
    // The locked amount is kept on the record so a backfill reproduces the books exactly
    result.margin = margin_acct ? need : 0.0;
    m_trade_list.push_back(result);

    // Update the current position record
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord position(
          stock, datetime, Null<Datetime>(), number, stoploss, goalPrice, number, money, cost.total,
          roundEx((realPrice - stoploss) * number * stock.unit(), precision), 0.0);
        position.buyCount = 1;
        if (margin_acct) {
            position.margin = need;
        }
        m_position[stock.id()] = position;
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += number;
        position.stoploss = stoploss;
        position.goalPrice = goalPrice;
        position.totalNumber += number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.openBasis = roundEx(position.openBasis + money, precision);
        position.settleBasis = roundEx(position.settleBasis + money, precision);
        position.totalCost = roundEx(cost.total + position.totalCost, precision);
        position.totalRisk =
          roundEx(position.totalRisk + (realPrice - stoploss) * number * stock.unit(), precision);
        if (margin_acct) {
            // Margin lots joining a cash-basis holding mix the two accountings in one record: the
            // whole position then follows the margin formula on a single averaged basis
            HKU_WARN_IF(position.margin == 0.0,
                        "{} {} Margin lots join a cash-basis holding, the books mix the two "
                        "accountings!",
                        datetime, stock.market_code());
            position.margin = roundEx(position.margin + need, precision);
        } else if (position.margin > 0.0) {
            // The other mixing direction: a cash-basis lot (placeholder or switched accounting)
            // joins margin lots and corrupts the same averaged basis
            HKU_WARN_IF(true,
                        "{} {} Cash lots join a margin-accounted holding, the books mix the two "
                        "accountings!",
                        datetime, stock.market_code());
        }
        position.buyCount++;
    }

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->buy(datetime, stock.market(), stock.code(), realPrice, number, stoploss, goalPrice,
                    from, remark);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::sell(const Datetime& datetime, const Stock& stock, price_t realPrice,
                               double number, price_t stoploss, price_t goalPrice,
                               price_t planPrice, SystemPart from, const string& remark) {
    TradeRecord result;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(!std::isfinite(realPrice) || realPrice < 0.0, result,
                        "{} {} Invalid sell real price({})!", datetime, stock.market_code(),
                        realPrice);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number == 0.0, result,
                        "{} {} number({}) must be finite and nonzero!", datetime,
                        stock.market_code(), number);

    // For the case where the dividend and the capital increase make the quantity not an integer
    // multiple of the minimum trade quantity, the whole position can only be sold with
    // number=MAX_DOUBLE
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Sell number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
                        "{} {} Sell number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

    // There is no position
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_TRACE_IF_RETURN(pos_iter == m_position.end(), result,
                        "{} {} This stock was not bought never! ({}, {:<.4f}, {}, {})", datetime,
                        stock.market_code(), datetime, realPrice, number, getSystemPartName(from));

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    PositionRecord& position = pos_iter->second;

    // Adjust the quantity to be sold; a sell quantity equal to MAX_DOUBLE means selling everything
    double real_number = number == MAX_DOUBLE ? position.number : number;

    // The quantity to be sold is greater than the current position quantity
    HKU_ERROR_IF_RETURN(position.number < real_number, result,
                        "{} {} Try to sell number({}) > number of position({})!", datetime,
                        stock.market_code(), real_number, position.number);

    CostRecord cost = getSellCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    // A margin-accounted holding returns only the released margin and the realized profit, the
    // notional never entered the cash when it was opened; the release is proportional to the
    // closed part of the holding. A realPrice 0 placeholder cannot be valued, and clearing the
    // margin without paying the release out would lose that money, so it is rejected up front
    HKU_WARN_IF_RETURN(
      position.margin > 0.0 && realPrice <= 0.0, result,
      "{} {} A margin-accounted holding can't be closed at a placeholder price(0)!", datetime,
      stock.market_code());
    price_t released = 0.0;
    // The cost notional and the settlement basis of the lots being closed, both released from the
    // books proportionally
    price_t basis = positionOpenBasis(position, real_number);
    price_t mtmBasis = positionSettleBasis(position, real_number);
    if (position.margin > 0.0) {
        released = roundEx(position.margin * (real_number / position.number), precision);
        // Both bases are money (price x number x unit), so the realized profit needs no unit
        // factor
        price_t realized = roundEx(money - mtmBasis, precision);
        m_cash = roundEx(m_cash + released + realized - cost.total, precision);
    } else {
        m_cash = roundEx(m_cash + money - cost.total, precision);
    }

    // Update the trade records
    result = TradeRecord(stock, datetime, BUSINESS_SELL, planPrice, realPrice, goalPrice,
                         real_number, cost, stoploss, m_cash, from, remark);
    m_trade_list.push_back(result);

    // Update the current position
    position.number -= real_number;
    position.margin = position.number > 0.0 ? roundEx(position.margin - released, precision) : 0.0;
    position.openBasis =
      position.number > 0.0 ? roundEx(position.openBasis - basis, precision) : 0.0;
    position.settleBasis =
      position.number > 0.0 ? roundEx(position.settleBasis - mtmBasis, precision) : 0.0;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    // position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);
    position.sellCount++;

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_position_history.push_back(position);
        // Delete the current position
        m_position.erase(stock.id());
    }

    // Return the loan if there is one
    if (getParam<bool>("support_borrow_cash") && m_borrow_cash > 0.0 && m_cash > 0.0) {
        price_t repay = m_borrow_cash < m_cash ? m_borrow_cash : m_cash;
        HKU_WARN_IF(!returnCash(datetime, repay),
                    "{} {} Failed to auto repay the borrowed cash({:<.4f})!", datetime,
                    stock.market_code(), repay);
    }

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->sell(datetime, stock.market(), stock.code(), realPrice, real_number, stoploss,
                     goalPrice, from, remark);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                    double number, price_t stoploss, price_t goalPrice,
                                    price_t planPrice, SystemPart from, const string& remark) {
    TradeRecord result;
    result.business = BUSINESS_INVALID;

    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(!std::isfinite(realPrice) || realPrice < 0.0, result,
                        "{} {} Invalid sell short real price({})!", datetime, stock.market_code(),
                        realPrice);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number == 0.0, result,
                        "{} {} number({}) must be finite and nonzero!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Buy number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                        "{} {} Buy number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());
    HKU_WARN_IF_RETURN(_isExpired(datetime, stock), result, "{} {} expired({}), can't sellShort!",
                       datetime, stock.market_code(), stock.lastDatetime());
    HKU_ERROR_IF_RETURN(
      stoploss != 0.0 && stoploss < realPrice, result,
      "{} {} Sell short's stoploss({:<.4f}) must be > realPrice({:<.4f}) or = 0! ", datetime,
      stock.market_code(), stoploss, realPrice);

    // Every margin model caps a short opening by the available margin, no matter whether the
    // account borrows securities automatically. The available funds are the net assets minus the
    // occupied margin, because a short opening adds the sale proceeds to the cash. Like in buy,
    // this gate must run before updateWithWeight to keep the weights from being applied twice,
    // and it is skipped for the legal realPrice=0 market-order placeholder (see buy)
    if (m_margin && m_margin->isMarginTrading(stock) && realPrice > 0.0) {
        // The margin of the short order itself, judged with a tolerance, same as buy
        price_t available = getAvailableMargin(datetime);
        price_t need = getOpenMargin(datetime, stock, realPrice, number, MarginSide::SHORT);
        double tolerance = std::max(1e-9, available * 1e-12);
        HKU_WARN_IF_RETURN(need > available + tolerance, result,
                           "{} {} Can't sellShort, needed margin({:<.4f}) > available({:<.4f})!",
                           datetime, stock.market_code(), need, available);
    }

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);
    // The short position grows at this datetime; drop the check frontier for this stock (see buy)
    m_tmp_weight_checked_until.erase(stock.id());

    int precision = getParam<int>("precision");

    // Resolve the executable quantity from the read-only borrow pool first, so a rejected or a
    // capped order never touches the debt/fee state. A short opening on a borrowable security may
    // need new borrowed securities; the borrow itself is committed only after every funding check
    // below passed
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    double sell_num = number;
    double borrow_num = 0.0;  // securities to borrow for this order
    // Securities that can not be lent out (futures, crypto, ...) book the short directly, without
    // the borrow/return accounting
    const bool borrow_asset = !m_margin || m_margin->needBorrowAsset(stock);
    if (borrow_asset) {
        if (getParam<bool>("support_borrow_stock")) {
            // Borrow on demand: the unsold stock already in the pool is used first, only the
            // shortfall creates new debt
            borrow_stock_map_type::const_iterator pool_iter = m_borrow_stock.find(stock.id());
            if (pool_iter != m_borrow_stock.end()) {
                double unsold = pool_iter->second.number;
                if (pos_iter != m_short_position.end()) {
                    unsold -= pos_iter->second.number;
                }
                // The short position must not exceed the borrowed quantity (an inconsistent
                // account); reject before borrowing so that no debt residue is left behind
                HKU_WARN_IF_RETURN(unsold < 0.0, result, "{} {} Borrowed Stock had all selled!",
                                   datetime, stock.market_code());
                if (unsold > 0.0) {
                    borrow_num = number > unsold ? number - unsold : 0.0;
                }
            } else {
                borrow_num = number;
            }
            // Borrowing needs a real price; a placeholder order can only use the existing pool
            HKU_WARN_IF_RETURN(borrow_num > 0.0 && realPrice <= 0.0, result,
                               "{} {} Can't borrow stock at a placeholder price(0)!", datetime,
                               stock.market_code());
        } else {
            // No automatic borrowing: the pool must already hold the securities
            borrow_stock_map_type::const_iterator bor_iter = m_borrow_stock.find(stock.id());
            HKU_ERROR_IF_RETURN(bor_iter == m_borrow_stock.end(), result,
                                "{} {} Non borrowed, can't sell short! ", datetime,
                                stock.market_code());

            double total_borrow_num = bor_iter->second.number;
            double can_sell_num = 0;
            if (pos_iter == m_short_position.end()) {
                // The borrowed stock has not been sold
                can_sell_num = total_borrow_num;
            } else {
                // The borrowed stock has been sold
                HKU_ERROR_IF_RETURN(pos_iter->second.number >= total_borrow_num, result,
                                    "{} {} Borrowed Stock had all selled!", datetime,
                                    stock.market_code());
                // The quantity that can be sold = the total borrowed - the already sold quantity
                can_sell_num = total_borrow_num - pos_iter->second.number;
            }

            // If the planned sell quantity is greater than the sellable quantity, sell the
            // sellable quantity
            if (number > can_sell_num) {
                sell_num = can_sell_num;
            }
        }
    }

    CostRecord cost = getSellCost(datetime, stock, realPrice, sell_num);
    // Gross proceeds on the position; fees only in totalCost (keeps totalProfit() exact)
    price_t gross = roundEx(realPrice * sell_num * stock.unit(), precision);
    price_t money = roundEx(gross - cost.total, precision);
    // The fee charged when new securities are borrowed, part of the funding that must be covered
    price_t borrow_stock_fee =
      borrow_num > 0.0 ? getBorrowStockCost(datetime, stock, realPrice, borrow_num).total : 0.0;

    // A margin-accounted short opening locks only the margin: the sale proceeds never enter the
    // cash, otherwise opening a short would create buying power out of nothing. The funding rule
    // mirrors buy: cover the shortfall (margin + sell fee + borrow fee) with new debt when the
    // account may borrow, otherwise the cash on hand must pay it. Like in buy, the post-borrow cash
    // is simulated before any debt exists, so a rejected order leaves no borrow residue behind
    bool margin_acct = isMarginAccounted(stock) && realPrice > 0.0;
    price_t need = margin_acct
                     ? getOpenMarginToLock(datetime, stock, realPrice, sell_num, MarginSide::SHORT)
                     : 0.0;
    // A model that reports no requirement at all can't fund the short by margin: the cash books
    // keep it instead of counting the full notional as equity for free
    HKU_WARN_IF(margin_acct && !(need > 0.0),
                "{} {} The margin model reported no requirement, the short opening is booked by "
                "cash!",
                datetime, stock.market_code());
    margin_acct = margin_acct && need > 0.0;
    price_t need_cash = 0.0;
    if (margin_acct) {
        need_cash = roundEx(need + cost.total + borrow_stock_fee, precision);
        price_t gap = roundEx(need_cash - m_cash, precision);
        if (gap > 0.0 && getParam<bool>("support_borrow_cash")) {
            bool covered = false;
            for (int i = 0; i < 32 && !covered; ++i) {
                price_t after =
                  roundEx(m_cash + gap - getBorrowCashCost(datetime, gap).total, precision);
                covered = after >= need_cash;
                if (!covered) {
                    price_t next = roundEx(gap + roundEx(need_cash - after, precision), precision);
                    // The cost rate must stay well below 100%; a non-increasing gap means the
                    // sequence stalled and the cost can never be covered
                    HKU_WARN_IF_RETURN(
                      next <= gap, result,
                      "{} {} Can't sellShort, the borrow cost can not be covered by gap({:<.4f})!",
                      datetime, stock.market_code(), gap);
                    gap = next;
                }
            }
            HKU_WARN_IF_RETURN(
              !covered, result,
              "{} {} Can't sellShort, the borrow cost can not be covered by gap({:<.4f})!",
              datetime, stock.market_code(), gap);
            borrowCash(datetime, gap);
        }
        HKU_WARN_IF_RETURN(m_cash < need_cash, result,
                           "{} {} Can't sellShort, need cash({:<.4f}) > current cash({:<.4f})!",
                           datetime, stock.market_code(), need_cash, m_cash);
    }

    // Commit the securities borrow after every check passed; its fee is deducted from the cash and
    // is already part of need_cash above. All borrowStock preconditions were validated in the
    // read-only step, so it can not fail here and leave a residue behind
    if (borrow_num > 0.0) {
        borrowStock(datetime, stock, realPrice, borrow_num);
    }

    // Update the cash: a margin opening pays the margin and the sell fee (the borrow-stock fee was
    // already charged by borrowStock), a cash opening receives the net proceeds
    m_cash = margin_acct ? roundEx(m_cash - need - cost.total, precision)
                         : roundEx(m_cash + money, precision);

    // Add it to the trade records
    result = TradeRecord(stock, datetime, BUSINESS_SELL_SHORT, planPrice, realPrice, goalPrice,
                         sell_num, cost, stoploss, m_cash, from, remark);
    // The locked amount is kept on the record so a backfill reproduces the books exactly
    result.margin = margin_acct ? need : 0.0;
    m_trade_list.push_back(result);

    // Update the current short position record
    price_t risk =
      stoploss > 0.0 ? roundEx((stoploss - realPrice) * sell_num * stock.unit(), precision) : 0.0;

    if (pos_iter == m_short_position.end()) {
        PositionRecord position(stock, datetime, Null<Datetime>(), sell_num, stoploss, goalPrice,
                                sell_num, 0.0, cost.total, risk, gross);
        position.sellCount = 1;
        // The gross opening proceeds of a short are its cost basis and its settlement basis (the
        // long side uses buyMoney)
        position.openBasis = gross;
        position.settleBasis = gross;
        if (margin_acct) {
            position.margin = need;
        }
        m_short_position[stock.id()] = position;
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += sell_num;
        position.stoploss = stoploss;
        position.goalPrice = goalPrice;
        position.totalNumber += sell_num;
        position.totalCost = roundEx(cost.total + position.totalCost, precision);
        position.totalRisk = roundEx(position.totalRisk + risk, precision);
        position.sellMoney = roundEx(position.sellMoney + gross, precision);
        position.openBasis = roundEx(position.openBasis + gross, precision);
        position.settleBasis = roundEx(position.settleBasis + gross, precision);
        if (margin_acct) {
            // Margin lots joining a cash-basis holding mix the two accountings in one record: the
            // whole position then follows the margin formula on a single averaged basis
            HKU_WARN_IF(position.margin == 0.0,
                        "{} {} Margin lots join a cash-basis holding, the books mix the two "
                        "accountings!",
                        datetime, stock.market_code());
            position.margin = roundEx(position.margin + need, precision);
        } else if (position.margin > 0.0) {
            // The other mixing direction: a cash-basis lot joins margin lots
            HKU_WARN_IF(true,
                        "{} {} Cash lots join a margin-accounted short, the books mix the two "
                        "accountings!",
                        datetime, stock.market_code());
        }
        position.sellCount++;
    }

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->sell(datetime, stock.market(), stock.code(), realPrice, sell_num, stoploss,
                     goalPrice, from, remark);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    _saveAction(result);

    return result;
}

TradeRecord TradeManager::buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                   double number, price_t stoploss, price_t goalPrice,
                                   price_t planPrice, SystemPart from, const string& remark) {
    TradeRecord result;
    HKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                        "{} {} datetime must be >= lastDatetime({})!", datetime,
                        stock.market_code(), lastDatetime());
    HKU_ERROR_IF_RETURN(!std::isfinite(realPrice) || realPrice < 0.0, result,
                        "{} {} Invalid buy short real price({})!", datetime, stock.market_code(),
                        realPrice);
    HKU_ERROR_IF_RETURN(!std::isfinite(number) || number == 0.0, result,
                        "{} {} number({}) must be finite and nonzero!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} buyShort number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
                        "{} {} buyShort number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

    // There is no short position
    position_map_type::iterator pos_iter = m_short_position.find(stock.id());
    HKU_WARN_IF_RETURN(pos_iter == m_short_position.end(), result,
                       "{} {} This stock was not sell never! ", datetime, stock.market_code());

    // Adjust the current position according to the ex-rights/ex-dividend information
    updateWithWeight(datetime);

    PositionRecord& position = pos_iter->second;

    // Adjust the quantity to be bought; a buy quantity equal to MAX_DOUBLE or greater than the
    // actual position means buying everything
    double real_number =
      (number == MAX_DOUBLE || number > position.number) ? position.number : number;

    CostRecord cost = getBuyCost(datetime, stock, realPrice, real_number);

    int precision = getParam<int>("precision");
    price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

    // The cash must cover the buyback and its cost in the plain books; a margin-accounted short
    // returns its released margin and the realized profit instead of paying the notional, and a
    // closing loss beyond the released margin may overdraw the cash like a long close does. A
    // realPrice 0 placeholder cannot be valued and would silently drop the margin, so reject it
    HKU_WARN_IF_RETURN(
      position.margin > 0.0 && realPrice <= 0.0, result,
      "{} {} A margin-accounted holding can't be closed at a placeholder price(0)!", datetime,
      stock.market_code());
    price_t released = 0.0;
    // The cost notional and the settlement basis of the lots being covered, both released from
    // the books proportionally
    price_t basis = positionOpenBasis(position, real_number);
    price_t mtmBasis = positionSettleBasis(position, real_number);
    if (position.margin > 0.0) {
        released = roundEx(position.margin * (real_number / position.number), precision);
        price_t realized = roundEx(mtmBasis - money, precision);
        m_cash = roundEx(m_cash + released + realized - cost.total, precision);
    } else {
        HKU_WARN_IF_RETURN(m_cash < roundEx(money + cost.total, precision), result,
                           "{} {} Can't buyShort, need cash({:<.4f}) > current cash({:<.4f})!",
                           datetime, stock.market_code(), roundEx(money + cost.total, precision),
                           m_cash);

        // Update the cash balance
        m_cash = roundEx(m_cash - money - cost.total, precision);
    }

    // Update the trade records
    result = TradeRecord(stock, datetime, BUSINESS_BUY_SHORT, planPrice, realPrice, goalPrice,
                         real_number, cost, stoploss, m_cash, from, remark);
    m_trade_list.push_back(result);

    // Scale the risk to the remaining quantity; a full close keeps the booked value for history
    double pre_number = position.number;
    position.number -= real_number;
    if (position.number > 0.0 && pre_number > 0.0) {
        position.totalRisk =
          roundEx(position.totalRisk * (position.number / pre_number), precision);
    }
    position.buyMoney = roundEx(position.buyMoney + money, precision);
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    // position.sellMoney = roundEx(position.sellMoney, precision);
    position.buyCount++;
    // Mirror _add_buy_short_tr: the covered part hands its margin back
    position.margin = position.number > 0.0 ? roundEx(position.margin - released, precision) : 0.0;
    position.openBasis =
      position.number > 0.0 ? roundEx(position.openBasis - basis, precision) : 0.0;
    position.settleBasis =
      position.number > 0.0 ? roundEx(position.settleBasis - mtmBasis, precision) : 0.0;

    if (position.number == 0) {
        position.cleanDatetime = datetime;
        m_short_position_history.push_back(position);
        // Delete the current position
        m_short_position.erase(stock.id());
    }

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->buy(datetime, stock.market(), stock.code(), realPrice, real_number, stoploss,
                    goalPrice, from, remark);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    if (getParam<bool>("support_borrow_stock") && (!m_margin || m_margin->needBorrowAsset(stock))) {
        returnStock(datetime, stock, realPrice, real_number);
    }

    // Return the loan last, after the stock-return fee: the repay must not starve the fee and fake
    // an overdraft the close never made
    if (getParam<bool>("support_borrow_cash") && m_borrow_cash > 0.0 && m_cash > 0.0) {
        price_t repay = m_borrow_cash < m_cash ? m_borrow_cash : m_cash;
        HKU_WARN_IF(!returnCash(datetime, repay),
                    "{} {} Failed to auto repay the borrowed cash({:<.4f})!", datetime,
                    stock.market_code(), repay);
    }

    _saveAction(result);

    return result;
}

price_t TradeManager::cash(const Datetime& datetime, KQuery::KType ktype) {
    // Gate on the persisted lastDatetime: m_last_update_datetime is not restored on load
    if (datetime == Null<Datetime>() || datetime == lastDatetime()) {
        return m_cash;
    }

    if (datetime > lastDatetime()) {
        updateWithWeight(datetime);
        return m_cash;
    }

    FundsRecord funds = getFunds(datetime, ktype);
    return funds.cash;
}

FundsRecord TradeManager::getFunds(KQuery::KType inktype) const {
    FundsRecord funds;
    int precision = getParam<int>("precision");

    string ktype(inktype);
    to_upper(ktype);

    price_t value{0.0};  // Current market value
    price_t occupied = 0.0;
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        auto price = record.stock.getMarketValue(lastDatetime(), ktype);
        if (record.margin > 0.0) {
            // A margin-accounted holding contributes its locked margin plus the unrealized profit
            value = roundEx((value + marginPositionEquity(record, price, true)), precision);
            occupied = roundEx(occupied + record.margin, precision);
        } else {
            value = roundEx((value + record.number * price * record.stock.unit()), precision);
        }
    }

    price_t short_value = 0.0;  // Current market value of the short position
    iter = m_short_position.begin();
    for (; iter != m_short_position.end(); ++iter) {
        const PositionRecord& record = iter->second;
        auto price = record.stock.getMarketValue(lastDatetime(), ktype);
        if (record.margin > 0.0) {
            // The locked margin is an asset on either side, the unrealized profit carries the
            // direction, so the contribution joins the long market value as well
            value = roundEx((value + marginPositionEquity(record, price, false)), precision);
            occupied = roundEx(occupied + record.margin, precision);
        } else {
            short_value =
              roundEx((short_value + record.number * price * record.stock.unit()), precision);
        }
    }
    funds.cash = m_cash;
    funds.market_value = value;
    funds.short_market_value = short_value;
    funds.occupied_margin = occupied;
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

    // Nothing to report before the account exists
    HKU_IF_RETURN(indatetime < m_init_datetime, funds);

    // When datetime is Null, return the cash in the current account and the funds occupied at the
    // buy, as well as the accumulated deposit and withdrawal funds
    HKU_IF_RETURN(indatetime == Null<Datetime>() || indatetime == lastDatetime(), getFunds(ktype));

    Datetime datetime(indatetime.year(), indatetime.month(), indatetime.day(), 23, 59);
    price_t market_value = 0.0;
    price_t short_market_value = 0.0;
    if (datetime > lastDatetime()) {
        // Adjust the position according to the ex-rights/ex-dividend data
        updateWithWeight(datetime);

        // When the query date is later than or equal to the last trade date, calculate the market
        // value of the currently held securities directly
        price_t occupied = 0.0;
        position_map_type::const_iterator iter = m_position.begin();
        for (; iter != m_position.end(); ++iter) {
            const PositionRecord& record = iter->second;
            price_t price = record.stock.getMarketValue(datetime, ktype);
            if (record.margin > 0.0) {
                market_value =
                  roundEx(market_value + marginPositionEquity(record, price, true), precision);
                occupied = roundEx(occupied + record.margin, precision);
            } else {
                market_value =
                  roundEx(market_value + price * record.number * record.stock.unit(), precision);
            }
        }

        iter = m_short_position.begin();
        for (; iter != m_short_position.end(); ++iter) {
            const PositionRecord& record = iter->second;
            price_t price = record.stock.getMarketValue(datetime, ktype);
            if (record.margin > 0.0) {
                market_value =
                  roundEx(market_value + marginPositionEquity(record, price, false), precision);
                occupied = roundEx(occupied + record.margin, precision);
            } else {
                short_market_value = roundEx(
                  short_market_value + price * record.number * record.stock.unit(), precision);
            }
        }

        funds.cash = m_cash;
        funds.market_value = market_value;
        funds.short_market_value = short_market_value;
        funds.occupied_margin = occupied;
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

    // When the query date is earlier than the last trade date, traverse the trade records and
    // calculate the market value and the cash of that day
    price_t cash = m_init_cash;
    struct Stock_Number {
        Stock_Number() : number(0) {}
        Stock_Number(const Stock& stock, double number) : stock(stock), number(number) {}

        Stock stock;
        double number;
        price_t margin{0.0};        // Margin locked by the holding, mirrors PositionRecord::margin
        price_t settle_basis{0.0};  // Notional basis of the held lots, mirrors PositionRecord
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
            // If the date of the trade record is later than the given date, break the loop; it is
            // done
            break;
        }

        cash = iter->cash;
        switch (iter->business) {
            case BUSINESS_INIT:
                checkin_cash += iter->realPrice;
                break;

            case BUSINESS_BUY: {
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter == stock_map.end()) {
                    stock_iter =
                      stock_map.insert({iter->stock.id(), Stock_Number(iter->stock, iter->number)})
                        .first;
                } else {
                    stock_iter->second.number += iter->number;
                }
                // Mirror the live margin accounting: the margin locked and the notional basis are
                // only meaningful for the holdings opened under the current settings
                Stock_Number& sn = stock_iter->second;
                sn.settle_basis = roundEx(
                  sn.settle_basis + iter->realPrice * iter->number * iter->stock.unit(), precision);
                // The record of a margin opening carries the amount it locked, otherwise the
                // model under the current settings decides; realPrice 0 is the market-order
                // placeholder and stays on the cash side
                if (iter->margin > 0.0) {
                    sn.margin = roundEx(sn.margin + iter->margin, precision);
                } else if (isMarginAccounted(iter->stock) && iter->realPrice > 0.0) {
                    sn.margin = roundEx(
                      sn.margin + getOpenMarginToLock(iter->datetime, iter->stock, iter->realPrice,
                                                      iter->number, MarginSide::LONG),
                      precision);
                }
                break;
            }

            case BUSINESS_GIFT:
            case BUSINESS_SUOGU: {
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter == stock_map.end()) {
                    stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
                } else {
                    // The free (gift) shares and the consolidation only change the quantity; the
                    // cost pool keeps its total, so the per-share basis dilutes or scales
                    stock_iter->second.number += iter->number;
                }
                break;
            }

            case BUSINESS_SELL: {
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter != stock_map.end()) {
                    Stock_Number& sn = stock_iter->second;
                    // Two steps like the live sell, so the remainder matches it to the last cent;
                    // a placeholder price never releases the margin, like the live rejection
                    price_t released = 0.0;
                    if (sn.margin > 0.0 && sn.number > 0.0) {
                        if (iter->realPrice <= 0.0) {
                            HKU_WARN(
                              "{} {} Margin holding closed at a placeholder price(0) in "
                              "m_trade_list, the locked margin stays!",
                              datetime, iter->stock.market_code());
                        } else {
                            released = roundEx(sn.margin * (iter->number / sn.number), precision);
                        }
                    }
                    // Release the basis proportionally before the quantity shrinks
                    price_t basis =
                      sn.number > 0.0 ? sn.settle_basis * (iter->number / sn.number) : 0.0;
                    sn.number -= iter->number;
                    sn.margin = sn.number > 0.0 ? roundEx(sn.margin - released, precision) : 0.0;
                    sn.settle_basis =
                      sn.number > 0.0 ? roundEx(sn.settle_basis - basis, precision) : 0.0;
                } else {
                    HKU_WARN("{} {} Sell error in m_trade_list!", datetime,
                             iter->stock.market_code());
                }
                break;
            }

            case BUSINESS_SELL_SHORT: {
                short_stock_iter = short_stock_map.find(iter->stock.id());
                if (short_stock_iter == short_stock_map.end()) {
                    short_stock_iter =
                      short_stock_map
                        .insert({iter->stock.id(), Stock_Number(iter->stock, iter->number)})
                        .first;
                } else {
                    short_stock_iter->second.number += iter->number;
                }
                Stock_Number& sn = short_stock_iter->second;
                sn.settle_basis = roundEx(
                  sn.settle_basis + iter->realPrice * iter->number * iter->stock.unit(), precision);
                if (iter->margin > 0.0) {
                    sn.margin = roundEx(sn.margin + iter->margin, precision);
                } else if (isMarginAccounted(iter->stock) && iter->realPrice > 0.0) {
                    sn.margin = roundEx(
                      sn.margin + getOpenMarginToLock(iter->datetime, iter->stock, iter->realPrice,
                                                      iter->number, MarginSide::SHORT),
                      precision);
                }
                break;
            }

            case BUSINESS_BUY_SHORT: {
                short_stock_iter = short_stock_map.find(iter->stock.id());
                if (short_stock_iter != short_stock_map.end()) {
                    Stock_Number& sn = short_stock_iter->second;
                    // Two steps like the live buyShort, so the remainder matches it to the last
                    // cent; a placeholder price never releases the margin, like the live rejection
                    price_t released = 0.0;
                    if (sn.margin > 0.0 && sn.number > 0.0) {
                        if (iter->realPrice <= 0.0) {
                            HKU_WARN(
                              "{} {} Margin short closed at a placeholder price(0) in "
                              "m_trade_list, the locked margin stays!",
                              datetime, iter->stock.market_code());
                        } else {
                            released = roundEx(sn.margin * (iter->number / sn.number), precision);
                        }
                    }
                    // Release the basis proportionally before the quantity shrinks
                    price_t basis =
                      sn.number > 0.0 ? sn.settle_basis * (iter->number / sn.number) : 0.0;
                    sn.number -= iter->number;
                    sn.margin = sn.number > 0.0 ? roundEx(sn.margin - released, precision) : 0.0;
                    sn.settle_basis =
                      sn.number > 0.0 ? roundEx(sn.settle_basis - basis, precision) : 0.0;
                } else {
                    HKU_WARN("{} {} BuyShort Error in m_trade_list!", datetime,
                             iter->stock.market_code());
                }
                break;
            }

            case BUSINESS_BORROW_ADJUST: {
                bor_stock_iter = bor_stock_map.find(iter->stock.id());
                if (bor_stock_iter != bor_stock_map.end()) {
                    BorrowRecord& bor = bor_stock_iter->second;
                    double total = 0.0;
                    list<BorrowRecord::Data>::iterator bor_iter = bor.record_list.begin();
                    for (; bor_iter != bor.record_list.end(); ++bor_iter) {
                        total += bor_iter->number;
                    }
                    // Scale the entries so that the sum follows the new quantity while the total
                    // value (price x number) stays invariant (borrow_asset is not touched)
                    if (total != 0.0) {
                        double factor = (total + iter->number) / total;
                        for (bor_iter = bor.record_list.begin(); bor_iter != bor.record_list.end();
                             ++bor_iter) {
                            bor_iter->number *= factor;
                            bor_iter->price /= factor;
                        }
                    }
                }
                break;
            }

            case BUSINESS_SHORT_ADJUST: {
                short_stock_iter = short_stock_map.find(iter->stock.id());
                if (short_stock_iter == short_stock_map.end()) {
                    short_stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
                } else {
                    // Both the gift and the consolidation arrive as a quantity change; the cost
                    // pool keeps its total
                    short_stock_iter->second.number += iter->number;
                }
                break;
            }

            case BUSINESS_BONUS:
            case BUSINESS_DIVIDEND_COMPENSATION:
                break;

            case BUSINESS_CHECKIN:
                checkin_cash += iter->realPrice;
                break;

            case BUSINESS_CHECKOUT:
                checkout_cash += iter->realPrice;
                break;

            case BUSINESS_CHECKIN_STOCK: {
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter == stock_map.end()) {
                    stock_iter =
                      stock_map.insert({iter->stock.id(), Stock_Number(iter->stock, iter->number)})
                        .first;
                } else {
                    stock_iter->second.number += iter->number;
                }
                // Mirrors the live check-in: the deposit adds its market value to the cost pool,
                // so the average opening price is unchanged
                stock_iter->second.settle_basis =
                  roundEx(stock_iter->second.settle_basis +
                            iter->realPrice * iter->number * iter->stock.unit(),
                          precision);
                checkin_stock = roundEx(
                  checkin_stock + iter->realPrice * iter->number * iter->stock.unit(), precision);
                break;
            }

            case BUSINESS_CHECKOUT_STOCK:
                stock_iter = stock_map.find(iter->stock.id());
                if (stock_iter != stock_map.end()) {
                    // The withdrawal releases the cost basis proportionally (mirrors checkoutStock)
                    Stock_Number& sn = stock_iter->second;
                    price_t basis =
                      sn.number > 0.0 ? sn.settle_basis * (iter->number / sn.number) : 0.0;
                    sn.number -= iter->number;
                    sn.settle_basis =
                      sn.number > 0.0 ? roundEx(sn.settle_basis - basis, precision) : 0.0;
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
                    double remain_num = iter->number;
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

            case BUSINESS_SETTLEMENT: {
                // The record carries the settled holding: the sign of the quantity is the side
                // (long > 0, short < 0), realPrice is the settlement price and margin is the lock
                // recomputed at it; the cash snapshot assigned above already carries the
                // mark-to-market posting
                auto& holding_map = iter->number >= 0.0 ? stock_map : short_stock_map;
                stock_iter = holding_map.find(iter->stock.id());
                if (stock_iter != holding_map.end()) {
                    Stock_Number& sn = stock_iter->second;
                    sn.settle_basis =
                      roundEx(iter->realPrice * sn.number * iter->stock.unit(), precision);
                    // The record always carries the lock snapshot: a zero demotes the holding to
                    // the cash books, exactly like the live settlement did
                    sn.margin = roundEx(std::max<price_t>(iter->margin, 0.0), precision);
                }
                break;
            }

            default:
                HKU_WARN("{} {} Unknown business in m_trade_list!", datetime,
                         iter->stock.market_code());
                break;
        }
    }

    // The settlement basis of the still-open holding (the opening notional until a settlement)
    auto settle_basis = [](const Stock_Number& sn) -> price_t { return sn.settle_basis; };

    price_t occupied = 0.0;
    stock_iter = stock_map.begin();
    for (; stock_iter != stock_map.end(); ++stock_iter) {
        const double& number = stock_iter->second.number;
        if (number == 0) {
            continue;
        }

        price_t price = stock_iter->second.stock.getMarketValue(datetime, ktype);
        if (stock_iter->second.margin > 0.0) {
            // A margin-accounted holding contributes its locked margin plus the unrealized profit
            market_value = roundEx(market_value + stock_iter->second.margin +
                                     (price * number * stock_iter->second.stock.unit() -
                                      settle_basis(stock_iter->second)),
                                   precision);
            occupied = roundEx(occupied + stock_iter->second.margin, precision);
        } else {
            market_value =
              roundEx(market_value + price * number * stock_iter->second.stock.unit(), precision);
        }
    }

    short_stock_iter = short_stock_map.begin();
    for (; short_stock_iter != short_stock_map.end(); ++short_stock_iter) {
        const double& number = short_stock_iter->second.number;
        if (number == 0) {
            continue;
        }

        price_t price = short_stock_iter->second.stock.getMarketValue(datetime, ktype);
        if (short_stock_iter->second.margin > 0.0) {
            market_value = roundEx(market_value + short_stock_iter->second.margin -
                                     (price * number * short_stock_iter->second.stock.unit() -
                                      settle_basis(short_stock_iter->second)),
                                   precision);
            occupied = roundEx(occupied + short_stock_iter->second.margin, precision);
        } else {
            short_market_value =
              roundEx(short_market_value + price * number * short_stock_iter->second.stock.unit(),
                      precision);
        }
    }

    funds.cash = cash;
    funds.market_value = market_value;
    funds.short_market_value = short_market_value;
    funds.occupied_margin = occupied;
    funds.base_cash = checkin_cash - checkout_cash;
    funds.base_asset = checkin_stock - checkout_stock;
    return funds;
}

FundsList TradeManager::getFundsList(const DatetimeList& dates, const KQuery::KType& ktype) {
    size_t total = dates.size();
    FundsList result(total);
    HKU_IF_RETURN(total == 0, result);

    // Without trade records, the replay below has nothing to do; delegate to the generic
    // per-date implementation (this also bypasses the Null lastDatetime comparisons)
    HKU_IF_RETURN(m_trade_list.empty(), TradeManagerBase::getFundsList(dates, ktype));

    // Process the dates in ascending order while keeping the input order in the result. The
    // maximum date is calculated first (it may advance the ex-rights data, i.e. state mutation),
    // exactly the same guarantee as the generic version pre-computing dates.back(); the branch
    // decisions of the remaining dates then use the updated lastDatetime(), and the replay
    // afterwards sees the final trade list.
    size_t max_pos = 0;
    for (size_t i = 1; i < total; ++i) {
        if (dates[max_pos] == Null<Datetime>() ||
            (dates[i] != Null<Datetime>() && dates[max_pos] < dates[i])) {
            max_pos = i;
        }
    }
    if (dates[max_pos] == Null<Datetime>()) {
        // All the dates are Null, only the current-state branch is hit
        for (size_t i = 0; i < total; ++i) {
            result[i] = getFunds(dates[i], ktype);
        }
        return result;
    }
    result[max_pos] = getFunds(dates[max_pos], ktype);
    Datetime last_dt = lastDatetime();

    std::vector<size_t> sorted_index(total);
    std::iota(sorted_index.begin(), sorted_index.end(), 0);
    std::stable_sort(sorted_index.begin(), sorted_index.end(),
                     [&dates](size_t a, size_t b) { return dates[a] < dates[b]; });

    std::vector<size_t> replay_pos;  // positions of the dates taking the replay branch
    for (size_t pos : sorted_index) {
        if (pos == max_pos) {
            continue;
        }

        const Datetime& d = dates[pos];
        if (d == Null<Datetime>() || d == last_dt || d < m_init_datetime) {
            result[pos] = getFunds(d, ktype);
            continue;
        }

        Datetime d_end(d.year(), d.month(), d.day(), 23, 59);
        if (d_end > last_dt) {
            result[pos] = getFunds(d, ktype);
        } else {
            replay_pos.push_back(pos);
        }
    }

    HKU_IF_RETURN(replay_pos.empty(), result);

    // Replay the trade list once and snapshot the holdings at each replay date. This is exactly
    // what the replay branch of getFunds does per date, but the whole list is traversed only
    // once: O(trades + dates x positions) instead of O(dates x trades).
    struct Holding {
        Stock stock;
        double number;
        price_t margin;  // locked margin, 0 for the plain cash holding
        price_t basis;   // notional basis of the held lots, used by the margin holdings only
    };
    struct DateSnapshot {
        price_t cash;
        price_t checkin_cash;
        price_t checkout_cash;
        price_t checkin_stock;
        price_t checkout_stock;
        price_t borrow_cash;
        price_t borrow_asset;
        std::vector<Holding> holdings;        // ascending by the stock id
        std::vector<Holding> short_holdings;  // ascending by the stock id
    };

    size_t num_replay = replay_pos.size();
    std::vector<DateSnapshot> snapshots(num_replay);

    // Same as the local accumulator struct of the getFunds replay branch
    struct StockNumber {
        StockNumber() : number(0), margin(0), settle_basis(0) {}
        StockNumber(const Stock& stock, double number)
        : stock(stock), number(number), margin(0), settle_basis(0) {}

        Stock stock;
        double number;
        price_t margin;        // locked margin, 0 for the plain cash holding
        price_t settle_basis;  // notional basis of the held lots, mirrors PositionRecord
    };

    int precision = getParam<int>("precision");
    price_t cash = m_init_cash;
    price_t checkin_cash = 0.0, checkout_cash = 0.0;
    price_t checkin_stock = 0.0, checkout_stock = 0.0;
    price_t borrow_cash = 0.0, borrow_asset = 0.0;
    std::map<uint64_t, StockNumber> stock_map;
    std::map<uint64_t, StockNumber> short_stock_map;
    std::map<uint64_t, BorrowRecord> bor_stock_map;
    // The remaining opening notional of the still-open holding; no rounding, same as settle_basis
    // above
    auto basis_of = [](const StockNumber& sn) -> price_t { return sn.settle_basis; };
    auto trade_iter = m_trade_list.begin();

    for (size_t r = 0; r < num_replay; ++r) {
        const Datetime& d = dates[replay_pos[r]];
        Datetime d_end(d.year(), d.month(), d.day(), 23, 59);

        for (; trade_iter != m_trade_list.end() && trade_iter->datetime <= d_end; ++trade_iter) {
            cash = trade_iter->cash;
            switch (trade_iter->business) {
                case BUSINESS_INIT:
                    checkin_cash += trade_iter->realPrice;
                    break;

                case BUSINESS_BUY: {
                    auto iter = stock_map.find(trade_iter->stock.id());
                    if (iter == stock_map.end()) {
                        iter = stock_map
                                 .emplace(trade_iter->stock.id(),
                                          StockNumber(trade_iter->stock, trade_iter->number))
                                 .first;
                    } else {
                        iter->second.number += trade_iter->number;
                    }
                    // The cost pool covers the held lots, so the average opening price stays the
                    // one of the live books
                    iter->second.settle_basis = roundEx(
                      iter->second.settle_basis +
                        trade_iter->realPrice * trade_iter->number * trade_iter->stock.unit(),
                      precision);
                    if (trade_iter->margin > 0.0) {
                        iter->second.margin =
                          roundEx(iter->second.margin + trade_iter->margin, precision);
                    } else if (isMarginAccounted(trade_iter->stock) &&
                               trade_iter->realPrice > 0.0) {
                        iter->second.margin =
                          roundEx(iter->second.margin +
                                    getOpenMarginToLock(trade_iter->datetime, trade_iter->stock,
                                                        trade_iter->realPrice, trade_iter->number,
                                                        MarginSide::LONG),
                                  precision);
                    }
                    break;
                }

                case BUSINESS_GIFT:
                case BUSINESS_SUOGU: {
                    auto iter = stock_map.find(trade_iter->stock.id());
                    if (iter == stock_map.end()) {
                        stock_map.emplace(trade_iter->stock.id(),
                                          StockNumber(trade_iter->stock, trade_iter->number));
                    } else {
                        // The free (gift) shares and the consolidation only change the quantity;
                        // the cost pool keeps its total
                        iter->second.number += trade_iter->number;
                    }
                    break;
                }

                case BUSINESS_SELL: {
                    auto iter = stock_map.find(trade_iter->stock.id());
                    if (iter != stock_map.end()) {
                        // Release the margin proportionally, in two steps like the live sell; a
                        // placeholder price never releases it, like the live rejection
                        price_t released = 0.0;
                        if (iter->second.margin > 0.0 && iter->second.number > 0.0) {
                            if (trade_iter->realPrice <= 0.0) {
                                HKU_WARN(
                                  "{} {} Margin holding closed at a placeholder price(0) "
                                  "in m_trade_list, the locked margin stays!",
                                  d, trade_iter->stock.market_code());
                            } else {
                                released = roundEx(
                                  iter->second.margin * (trade_iter->number / iter->second.number),
                                  precision);
                            }
                        }
                        // Release the basis proportionally before the quantity shrinks
                        price_t basis =
                          iter->second.number > 0.0
                            ? iter->second.settle_basis * (trade_iter->number / iter->second.number)
                            : 0.0;
                        iter->second.number -= trade_iter->number;
                        iter->second.margin = iter->second.number > 0.0
                                                ? roundEx(iter->second.margin - released, precision)
                                                : 0.0;
                        iter->second.settle_basis =
                          iter->second.number > 0.0
                            ? roundEx(iter->second.settle_basis - basis, precision)
                            : 0.0;
                    } else {
                        HKU_WARN("{} {} Sell error in m_trade_list!", d,
                                 trade_iter->stock.market_code());
                    }
                    break;
                }

                case BUSINESS_SELL_SHORT: {
                    auto iter = short_stock_map.find(trade_iter->stock.id());
                    if (iter == short_stock_map.end()) {
                        iter = short_stock_map
                                 .emplace(trade_iter->stock.id(),
                                          StockNumber(trade_iter->stock, trade_iter->number))
                                 .first;
                    } else {
                        iter->second.number += trade_iter->number;
                    }
                    iter->second.settle_basis = roundEx(
                      iter->second.settle_basis +
                        trade_iter->realPrice * trade_iter->number * trade_iter->stock.unit(),
                      precision);
                    if (trade_iter->margin > 0.0) {
                        iter->second.margin =
                          roundEx(iter->second.margin + trade_iter->margin, precision);
                    } else if (isMarginAccounted(trade_iter->stock) &&
                               trade_iter->realPrice > 0.0) {
                        iter->second.margin =
                          roundEx(iter->second.margin +
                                    getOpenMarginToLock(trade_iter->datetime, trade_iter->stock,
                                                        trade_iter->realPrice, trade_iter->number,
                                                        MarginSide::SHORT),
                                  precision);
                    }
                    break;
                }

                case BUSINESS_BUY_SHORT: {
                    auto iter = short_stock_map.find(trade_iter->stock.id());
                    if (iter != short_stock_map.end()) {
                        // Release the margin proportionally, in two steps like the live buyShort;
                        // a placeholder price never releases it, like the live rejection
                        price_t released = 0.0;
                        if (iter->second.margin > 0.0 && iter->second.number > 0.0) {
                            if (trade_iter->realPrice <= 0.0) {
                                HKU_WARN(
                                  "{} {} Margin short closed at a placeholder price(0) in "
                                  "m_trade_list, the locked margin stays!",
                                  d, trade_iter->stock.market_code());
                            } else {
                                released = roundEx(
                                  iter->second.margin * (trade_iter->number / iter->second.number),
                                  precision);
                            }
                        }
                        // Release the basis proportionally before the quantity shrinks
                        price_t basis =
                          iter->second.number > 0.0
                            ? iter->second.settle_basis * (trade_iter->number / iter->second.number)
                            : 0.0;
                        iter->second.number -= trade_iter->number;
                        iter->second.margin = iter->second.number > 0.0
                                                ? roundEx(iter->second.margin - released, precision)
                                                : 0.0;
                        iter->second.settle_basis =
                          iter->second.number > 0.0
                            ? roundEx(iter->second.settle_basis - basis, precision)
                            : 0.0;
                    } else {
                        HKU_WARN("{} {} BuyShort Error in m_trade_list!", d,
                                 trade_iter->stock.market_code());
                    }
                    break;
                }

                case BUSINESS_BORROW_ADJUST: {
                    auto iter = bor_stock_map.find(trade_iter->stock.id());
                    if (iter != bor_stock_map.end()) {
                        BorrowRecord& bor = iter->second;
                        double total = 0.0;
                        for (auto& entry : bor.record_list) {
                            total += entry.number;
                        }
                        // Scale the entries so that the sum follows the new quantity while the
                        // total value (price x number) stays invariant
                        if (total != 0.0) {
                            double factor = (total + trade_iter->number) / total;
                            for (auto& entry : bor.record_list) {
                                entry.number *= factor;
                                entry.price /= factor;
                            }
                        }
                    }
                    break;
                }

                case BUSINESS_SHORT_ADJUST: {
                    auto iter = short_stock_map.find(trade_iter->stock.id());
                    if (iter == short_stock_map.end()) {
                        iter = short_stock_map
                                 .emplace(trade_iter->stock.id(),
                                          StockNumber(trade_iter->stock, trade_iter->number))
                                 .first;
                    } else {
                        // Both the gift and the consolidation arrive as a quantity change; the
                        // cost pool keeps its total
                        iter->second.number += trade_iter->number;
                    }
                    break;
                }

                case BUSINESS_BONUS:
                case BUSINESS_DIVIDEND_COMPENSATION:
                    break;

                case BUSINESS_CHECKIN:
                    checkin_cash += trade_iter->realPrice;
                    break;

                case BUSINESS_CHECKOUT:
                    checkout_cash += trade_iter->realPrice;
                    break;

                case BUSINESS_CHECKIN_STOCK: {
                    auto iter = stock_map.find(trade_iter->stock.id());
                    if (iter == stock_map.end()) {
                        iter = stock_map
                                 .emplace(trade_iter->stock.id(),
                                          StockNumber(trade_iter->stock, trade_iter->number))
                                 .first;
                    } else {
                        iter->second.number += trade_iter->number;
                    }
                    // The deposit adds its market value to the cost pool, so the average opening
                    // price is unchanged
                    iter->second.settle_basis = roundEx(
                      iter->second.settle_basis +
                        trade_iter->realPrice * trade_iter->number * trade_iter->stock.unit(),
                      precision);
                    checkin_stock =
                      roundEx(checkin_stock + trade_iter->realPrice * trade_iter->number *
                                                trade_iter->stock.unit(),
                              precision);
                    break;
                }

                case BUSINESS_CHECKOUT_STOCK: {
                    auto iter = stock_map.find(trade_iter->stock.id());
                    if (iter != stock_map.end()) {
                        // The withdrawal releases the cost basis proportionally (mirrors the live
                        // checkoutStock)
                        price_t basis =
                          iter->second.number > 0.0
                            ? iter->second.settle_basis * (trade_iter->number / iter->second.number)
                            : 0.0;
                        iter->second.number -= trade_iter->number;
                        iter->second.settle_basis =
                          iter->second.number > 0.0
                            ? roundEx(iter->second.settle_basis - basis, precision)
                            : 0.0;
                    } else {
                        HKU_WARN("{} {} CheckoutStock Error in m_trade_list!", d,
                                 trade_iter->stock.market_code());
                    }
                    checkout_stock =
                      roundEx(checkout_stock + trade_iter->realPrice * trade_iter->number *
                                                 trade_iter->stock.unit(),
                              precision);
                    break;
                }

                case BUSINESS_BORROW_CASH:
                    borrow_cash += trade_iter->realPrice;
                    break;

                case BUSINESS_RETURN_CASH:
                    borrow_cash -= trade_iter->realPrice;
                    break;

                case BUSINESS_BORROW_STOCK: {
                    borrow_asset =
                      roundEx(borrow_asset + trade_iter->realPrice * trade_iter->number *
                                               trade_iter->stock.unit(),
                              precision);
                    auto iter = bor_stock_map.find(trade_iter->stock.id());
                    if (iter == bor_stock_map.end()) {
                        BorrowRecord bor;
                        BorrowRecord::Data data(trade_iter->datetime, trade_iter->realPrice,
                                                trade_iter->number);
                        bor.record_list.push_back(data);
                        bor_stock_map.emplace(trade_iter->stock.id(), std::move(bor));
                    } else {
                        BorrowRecord::Data data(trade_iter->datetime, trade_iter->realPrice,
                                                trade_iter->number);
                        iter->second.record_list.push_back(data);
                    }
                    break;
                }

                case BUSINESS_RETURN_STOCK: {
                    auto iter = bor_stock_map.find(trade_iter->stock.id());
                    if (iter == bor_stock_map.end()) {
                        HKU_WARN("{} {} Error return stock in m_trade_list!", trade_iter->datetime,
                                 trade_iter->stock.market_code());
                    } else {
                        BorrowRecord& bor = iter->second;
                        double remain_num = trade_iter->number;
                        do {
                            auto bor_iter = bor.record_list.begin();
                            if (remain_num == bor_iter->number) {
                                borrow_asset -=
                                  roundEx(bor_iter->price * remain_num * trade_iter->stock.unit(),
                                          precision);
                                bor.record_list.pop_front();
                                break;

                            } else if (remain_num < bor_iter->number) {
                                borrow_asset -=
                                  roundEx(bor_iter->price * remain_num * trade_iter->stock.unit(),
                                          precision);
                                bor_iter->number -= remain_num;
                                break;

                            } else {  // remain_num > bor_iter->number
                                borrow_asset -= roundEx(
                                  bor_iter->price * bor_iter->number * trade_iter->stock.unit(),
                                  precision);
                                remain_num -= bor_iter->number;
                                bor.record_list.pop_front();
                            }
                        } while (!bor.record_list.empty());

                        if (bor.record_list.empty()) {
                            bor_stock_map.erase(iter);
                        }
                    }
                    break;
                }

                case BUSINESS_SETTLEMENT: {
                    // Same as the replay of getFunds: the sign of the quantity is the side, the
                    // price is the settlement price and the margin is the lock recomputed at it
                    auto& holding_map = trade_iter->number >= 0.0 ? stock_map : short_stock_map;
                    auto iter = holding_map.find(trade_iter->stock.id());
                    if (iter != holding_map.end()) {
                        iter->second.settle_basis = roundEx(
                          trade_iter->realPrice * iter->second.number * trade_iter->stock.unit(),
                          precision);
                        if (trade_iter->margin > 0.0) {
                            iter->second.margin = roundEx(trade_iter->margin, precision);
                        }
                    }
                    break;
                }

                default:
                    HKU_WARN("{} {} Unknown business in m_trade_list!", d,
                             trade_iter->stock.market_code());
                    break;
            }
        }

        // Snapshot the current accumulators; the map iteration order (ascending by the stock id)
        // is preserved, so the market value accumulation below is identical to the per-date
        // replay of getFunds
        DateSnapshot& snap = snapshots[r];
        snap.cash = cash;
        snap.checkin_cash = checkin_cash;
        snap.checkout_cash = checkout_cash;
        snap.checkin_stock = checkin_stock;
        snap.checkout_stock = checkout_stock;
        snap.borrow_cash = borrow_cash;
        snap.borrow_asset = borrow_asset;
        for (const auto& kv : stock_map) {
            snap.holdings.push_back(
              Holding{kv.second.stock, kv.second.number, kv.second.margin, basis_of(kv.second)});
        }
        for (const auto& kv : short_stock_map) {
            snap.short_holdings.push_back(
              Holding{kv.second.stock, kv.second.number, kv.second.margin, basis_of(kv.second)});
        }
    }

    // The market value of each date is independent; calculate them in parallel, mirroring the
    // per-date market value computation of the getFunds replay branch
    global_parallel_for_index_void(0, num_replay, [&](size_t r) {
        const DateSnapshot& snap = snapshots[r];
        const Datetime& d = dates[replay_pos[r]];
        Datetime d_end(d.year(), d.month(), d.day(), 23, 59);

        FundsRecord funds;
        price_t market_value = 0.0;
        price_t occupied = 0.0;
        for (const auto& holding : snap.holdings) {
            if (holding.number == 0) {
                continue;
            }
            price_t price = holding.stock.getMarketValue(d_end, ktype);
            if (holding.margin > 0.0) {
                // A margin-accounted holding contributes its locked margin plus the unrealized
                // profit, exactly like the getFunds replay branch
                market_value =
                  roundEx(market_value + holding.margin +
                            (price * holding.number * holding.stock.unit() - holding.basis),
                          precision);
                occupied = roundEx(occupied + holding.margin, precision);
            } else {
                market_value =
                  roundEx(market_value + price * holding.number * holding.stock.unit(), precision);
            }
        }

        price_t short_market_value = 0.0;
        for (const auto& holding : snap.short_holdings) {
            if (holding.number == 0) {
                continue;
            }
            price_t price = holding.stock.getMarketValue(d_end, ktype);
            if (holding.margin > 0.0) {
                // The locked margin of a short is an asset as well: its unrealized profit already
                // carries the sign, so it goes into market_value and never into the liability side
                market_value =
                  roundEx(market_value + holding.margin -
                            (price * holding.number * holding.stock.unit() - holding.basis),
                          precision);
                occupied = roundEx(occupied + holding.margin, precision);
            } else {
                short_market_value = roundEx(
                  short_market_value + price * holding.number * holding.stock.unit(), precision);
            }
        }

        funds.cash = snap.cash;
        funds.market_value = market_value;
        funds.short_market_value = short_market_value;
        funds.base_cash = snap.checkin_cash - snap.checkout_cash;
        funds.base_asset = snap.checkin_stock - snap.checkout_stock;
        funds.borrow_cash = snap.borrow_cash;
        funds.borrow_asset = snap.borrow_asset;
        funds.occupied_margin = occupied;
        result[replay_pos[r]] = funds;
    });

    return result;
}

/******************************************************************************
 *  Every time a trade operation is executed, adjust the held position and the cash records
 * according to the ex-rights/ex-dividend information first A lazy update strategy is adopted, i.e.
 * the current position and the assets are updated only when the current position information is
 * needed or a sell happens Input parameter: the date of this operation History: 1) added on
 * 2009/12/22
 *****************************************************************************/
void TradeManager::updateWithWeight(const Datetime& datetime) {
    HKU_IF_RETURN(datetime <= m_last_update_datetime, void());

    // Query date range of the ex-rights/ex-dividend information
    Datetime start_date(lastDatetime().date() + bd::days(1));
    Datetime end_date(datetime.date() + bd::days(1));

    // Incremental short-circuit: a stock's ex-rights list is only scanned for the date range not
    // already checked on prior updates. m_tmp_weight_checked_until is only written after all three
    // loops below complete, so a stock appearing in more than one map still uses the same window
    auto scan_start = [&](uint64_t stock_id) {
        unordered_map<uint64_t, Datetime>::const_iterator cached =
          m_tmp_weight_checked_until.find(stock_id);
        if (cached != m_tmp_weight_checked_until.end() && cached->second > start_date) {
            return cached->second;
        }
        return start_date;
    };

    int precision = getParam<int>("precision");
    TradeRecordList new_trade_buffer;

    // Account cash before this batch's dividends, used as the replay base below
    price_t cash_base = m_cash;

    // Update the position information and cache the newly added trade records
    position_map_type::iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        PositionRecord& position = position_iter->second;
        Stock stock = position.stock;

        Datetime s = scan_start(stock.id());
        if (s >= end_date)
            continue;
        StockWeightList weights = stock.getWeight(s, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for (; weight_iter != weights.end(); ++weight_iter) {
            // Skip it when there is no dividend and the numbers of the bonus shares and the
            // capitalized shares are both zero
            if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
                0.0 == weight_iter->increasement() && 0.0 == weight_iter->suogu()) {
                continue;
            }

            // It must be done before the bonus shares are added, because the bonus shares change
            // the position quantity
            if (weight_iter->bonus() != 0.0) {
                price_t bonus = roundEx(position.number * weight_iter->bonus() * 0.1, precision);
                position.sellMoney = roundEx(position.sellMoney + bonus, precision);
                m_cash = roundEx(m_cash + bonus, precision);

                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_BONUS, bonus, bonus,
                                   0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            double addcount =
              (position.number / 10.0) * (weight_iter->countAsGift() + weight_iter->increasement());
            if (addcount != 0.0) {
                // A gift only rescales the quantity: scaling the accumulated number by the same
                // ratio keeps the averaged cost on the new price, and unlike "+= addcount" it
                // stays right after a partial sale (sell() leaves totalNumber untouched)
                const double old_number = position.number;
                position.number += addcount;
                if (old_number > 0.0) {
                    position.totalNumber =
                      roundUp(position.totalNumber * (position.number / old_number), 0);
                }
                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_GIFT, 0.0, 0.0, 0.0,
                                   addcount, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            if (weight_iter->suogu() > 0.0) {
                const double old_number = position.number;
                double suogu_number = old_number * weight_iter->suogu();
                double change_number = 0.0;
                if (suogu_number < old_number) {
                    // The share contraction adopts the round-up
                    position.number = roundUp(suogu_number, 0);
                    change_number = position.number - old_number;
                } else if (suogu_number > old_number) {
                    // The share expansion adopts the truncation
                    position.number = roundDown(suogu_number, 0);
                    change_number = position.number - old_number;
                }

                if (change_number != 0.0) {
                    // Scale the accumulated quantity by the same ratio: sell() leaves totalNumber
                    // untouched, so adding the change alone would distort the averaged cost
                    // buyMoney / totalNumber once part of the holding was sold
                    if (old_number > 0.0) {
                        position.totalNumber =
                          roundUp(position.totalNumber * (position.number / old_number), 0);
                    }
                    TradeRecord record(stock, weight_iter->datetime(), BUSINESS_SUOGU, 0.0, 0.0,
                                       0.0, change_number, CostRecord(), 0.0, m_cash, PART_INVALID);
                    new_trade_buffer.push_back(record);
                }
            }

        } /* for weight */
    } /* for position */

    // A-share margin trading: the borrowed stock must follow the ex-rights events as well. The
    // cash dividend is compensated to the lender (a cash outflow), and the bonus/consolidation
    // events scale the owed quantity by the same ratio, mirroring the long position above.
    borrow_stock_map_type::iterator borrow_iter = m_borrow_stock.begin();
    for (; borrow_iter != m_borrow_stock.end(); ++borrow_iter) {
        BorrowRecord& borrow = borrow_iter->second;
        Stock stock = borrow.stock;

        Datetime s = scan_start(stock.id());
        if (s >= end_date)
            continue;
        StockWeightList weights = stock.getWeight(s, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for (; weight_iter != weights.end(); ++weight_iter) {
            if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
                0.0 == weight_iter->increasement() && 0.0 == weight_iter->suogu()) {
                continue;
            }

            // The cash dividend is compensated to the lender: a cash outflow from the account
            // (a financing cost of the short position, not a negative dividend income)
            if (weight_iter->bonus() != 0.0) {
                price_t bonus = roundEx(borrow.number * weight_iter->bonus() * 0.1, precision);
                m_cash = roundEx(m_cash - bonus, precision);
                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_DIVIDEND_COMPENSATION,
                                   bonus, bonus, 0.0, 0, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            double addcount =
              (borrow.number / 10.0) * (weight_iter->countAsGift() + weight_iter->increasement());
            if (addcount != 0.0) {
                double new_number = borrow.number + addcount;
                double factor = new_number / borrow.number;
                for (auto& entry : borrow.record_list) {  // keep the sum and the value invariant
                    entry.number *= factor;
                    entry.price /= factor;
                }
                borrow.number = new_number;
                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_BORROW_ADJUST, 0.0, 0.0,
                                   0.0, addcount, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            if (weight_iter->suogu() > 0.0) {
                double suogu_number = borrow.number * weight_iter->suogu();
                double new_number = borrow.number;
                if (suogu_number < borrow.number) {
                    // The share contraction adopts the round-up
                    new_number = roundUp(suogu_number, 0);
                } else if (suogu_number > borrow.number) {
                    // The share expansion adopts the truncation
                    new_number = roundDown(suogu_number, 0);
                }

                double change_number = new_number - borrow.number;
                if (change_number != 0.0) {
                    double factor = new_number / borrow.number;
                    for (auto& entry :
                         borrow.record_list) {  // keep the sum and the value invariant
                        entry.number *= factor;
                        entry.price /= factor;
                    }
                    borrow.number = new_number;
                    TradeRecord record(stock, weight_iter->datetime(), BUSINESS_BORROW_ADJUST, 0.0,
                                       0.0, 0.0, change_number, CostRecord(), 0.0, m_cash,
                                       PART_INVALID);
                    new_trade_buffer.push_back(record);
                }
            }
        } /* for weight */
    } /* for borrow stock */

    // The short position follows the same bonus/consolidation ratio so that it stays in sync
    // with the borrowed quantity (buyShort covers it by the current position number)
    position_map_type::iterator short_iter = m_short_position.begin();
    for (; short_iter != m_short_position.end(); ++short_iter) {
        PositionRecord& position = short_iter->second;
        Stock stock = position.stock;

        Datetime s = scan_start(stock.id());
        if (s >= end_date)
            continue;
        StockWeightList weights = stock.getWeight(s, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for (; weight_iter != weights.end(); ++weight_iter) {
            if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
                0.0 == weight_iter->increasement() && 0.0 == weight_iter->suogu()) {
                continue;
            }

            double addcount =
              (position.number / 10.0) * (weight_iter->countAsGift() + weight_iter->increasement());
            if (addcount != 0.0) {
                // Same ratio scaling as the long side: the short average cost follows the halved
                // price after a gift even when part of the short was already covered
                const double old_number = position.number;
                position.number += addcount;
                if (old_number > 0.0) {
                    position.totalNumber =
                      roundUp(position.totalNumber * (position.number / old_number), 0);
                }
                TradeRecord record(stock, weight_iter->datetime(), BUSINESS_SHORT_ADJUST, 0.0, 0.0,
                                   0.0, addcount, CostRecord(), 0.0, m_cash, PART_INVALID);
                new_trade_buffer.push_back(record);
            }

            if (weight_iter->suogu() > 0.0) {
                const double old_number = position.number;
                double suogu_number = old_number * weight_iter->suogu();
                double change_number = 0.0;
                if (suogu_number < old_number) {
                    position.number = roundUp(suogu_number, 0);
                    change_number = position.number - old_number;
                } else if (suogu_number > old_number) {
                    position.number = roundDown(suogu_number, 0);
                    change_number = position.number - old_number;
                }

                if (change_number != 0.0) {
                    // Keep the short average cost (sellMoney / totalNumber) on the scaled price
                    // too, scaling by the same ratio for the same partial-cover reason as the long
                    // side
                    if (old_number > 0.0) {
                        position.totalNumber =
                          roundUp(position.totalNumber * (position.number / old_number), 0);
                    }
                    TradeRecord record(stock, weight_iter->datetime(), BUSINESS_SHORT_ADJUST, 0.0,
                                       0.0, 0.0, change_number, CostRecord(), 0.0, m_cash,
                                       PART_INVALID);
                    new_trade_buffer.push_back(record);
                }
            }
        } /* for weight */
    } /* for short position */

    // Sort by datetime, then replay from cash_base so each record's cash matches its own time.
    // Only BONUS changes cash; stable_sort keeps a bonus ahead of same-date gift records.
    std::stable_sort(
      new_trade_buffer.begin(), new_trade_buffer.end(),
      std::bind(std::less<Datetime>(), std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

    size_t total = new_trade_buffer.size();
    price_t running_cash = cash_base;
    for (size_t i = 0; i < total; ++i) {
        if (new_trade_buffer[i].business == BUSINESS_BONUS) {
            running_cash = roundEx(running_cash + new_trade_buffer[i].realPrice, precision);
        } else if (new_trade_buffer[i].business == BUSINESS_DIVIDEND_COMPENSATION) {
            running_cash = roundEx(running_cash - new_trade_buffer[i].realPrice, precision);
        }
        new_trade_buffer[i].cash = running_cash;
    }

    for (size_t i = 0; i < total; ++i) {
        m_trade_list.push_back(new_trade_buffer[i]);
    }

    // Advance the per-stock incremental check pointer for every stock visited this update; the
    // frontier only ever moves forward so an out-of-order update can never rewind it
    auto advance_frontier = [&](uint64_t id) {
        auto it = m_tmp_weight_checked_until.find(id);
        if (it == m_tmp_weight_checked_until.end()) {
            m_tmp_weight_checked_until[id] = end_date;
        } else if (end_date > it->second) {
            it->second = end_date;  // only ever move forward
        }
    };
    for (const auto& kv : m_position)
        advance_frontier(kv.first);
    for (const auto& kv : m_borrow_stock)
        advance_frontier(kv.first);
    for (const auto& kv : m_short_position)
        advance_frontier(kv.first);

    m_last_update_datetime = datetime;
}

bool TradeManager::isSettled(const Datetime& datetime) const {
    HKU_IF_RETURN(datetime.isNull(), false);
    // The trade list is chronologically ordered, so the probe stops at the first older record
    for (auto iter = m_trade_list.rbegin(); iter != m_trade_list.rend(); ++iter) {
        if (iter->datetime < datetime) {
            break;
        }
        if (iter->business == BUSINESS_SETTLEMENT && iter->datetime == datetime) {
            return true;
        }
    }
    return false;
}

bool TradeManager::isForcedLiquidationQueued(const Datetime& datetime) const {
    HKU_IF_RETURN(m_tmp_force_liquidate_date.isNull(), false);
    HKU_IF_RETURN(!datetime.isNull() && datetime < m_tmp_force_liquidate_date, false);
    // Consumed once the flattening is done: only an armed queue with something left to close is
    // pending, which also keeps a flat account from being flagged again by a stale arming
    return !m_position.empty() || !m_short_position.empty();
}

bool TradeManager::settlement(const Datetime& datetime, KQuery::KType ktype) {
    HKU_IF_RETURN(datetime.isNull(), false);
    // Only the margin books settle a day; the plain cash books keep their live valuation
    HKU_IF_RETURN(!tryGetParam<bool>("margin_accounting", false), false);
    HKU_IF_RETURN(!tryGetParam<bool>("settle_with_close", true), false);
    HKU_IF_RETURN(!m_margin, false);
    HKU_WARN_IF_RETURN(datetime < lastDatetime(), false,
                       "{} settlement must be >= lastDatetime({})!", datetime, lastDatetime());
    // Idempotent: a second call of the same moment must not post the profit twice
    HKU_IF_RETURN(isSettled(datetime), true);

    // The ex-rights adjustment must land before the basis is marked: a gift or a consolidation
    // changes the quantity while the basis keeps its total, so scaling it afterwards would book
    // the whole price move as a false profit
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t maintain_required = 0.0;
    bool posted = false;

    // The settlement price of a date without a bar (a suspended instrument) falls back to its last
    // valid close: getMarketValue already does it, the probe only tags the record
    auto settle_price = [&](const Stock& stock, string& remark) -> price_t {
        KRecord record = stock.getKRecord(datetime, ktype);
        if (record.datetime != datetime) {
            remark = "no bar, the last close is used";
            HKU_DEBUG("{} {} has no bar, the settlement price falls back to its last close",
                      datetime, stock.market_code());
        }
        return stock.getMarketValue(datetime, ktype);
    };

    // Post one margin holding: the mark-to-market profit into the cash, the basis to the
    // settlement price, then the lock recomputed at that price with the difference posted
    auto settle_holding = [&](PositionRecord& position, bool is_long) {
        HKU_IF_RETURN(!(position.number > 0.0) || !(position.margin > 0.0), void());
        const Stock& stock = position.stock;
        // A holding past its last registered bar is owned by the expiry liquidation, not by the
        // daily settlement
        const Datetime& last = stock.lastDatetime();
        HKU_IF_RETURN(!last.isNull() && datetime > last, void());

        string remark;
        price_t price = settle_price(stock, remark);
        HKU_WARN_IF_RETURN(!(price > 0.0), void(),
                           "{} {}: no settlement price, the holding is not settled!", datetime,
                           stock.market_code());

        // Both bases are money totals, so the unit is already inside and neither side needs a
        // direction factor beyond the sign of the price move
        price_t mv = roundEx(price * position.number * stock.unit(), precision);
        price_t basis = roundEx(positionSettleBasis(position, position.number), precision);
        m_cash = roundEx(m_cash + (is_long ? mv - basis : basis - mv), precision);
        position.settleBasis = mv;

        // The lock is recomputed on the settlement price; an increase is financed like an opening,
        // and an account already in a margin call may go overdrawn instead of failing to settle
        MarginSide side = is_long ? MarginSide::LONG : MarginSide::SHORT;
        price_t relock = getOpenMarginToLock(datetime, stock, price, position.number, side);
        price_t delta = roundEx(relock - position.margin, precision);
        if (delta > 0.0 && m_cash < delta) {
            HKU_WARN_IF(!_borrowForAdd(datetime, delta),
                        "{} {}: can't finance the margin increase({:<.4f}), the account goes "
                        "overdrawn!",
                        datetime, stock.market_code(), delta);
        }
        m_cash = roundEx(m_cash - delta, precision);
        position.margin = relock;

        // One record per settled holding: the sign of the quantity carries the side, because one
        // security may be held on both sides and the replay has to tell them apart
        TradeRecord record(stock, datetime, BUSINESS_SETTLEMENT, price, price, 0.0,
                           is_long ? position.number : -position.number, CostRecord(), 0.0, m_cash,
                           PART_INVALID, remark);
        record.margin = relock;
        m_trade_list.push_back(record);
        posted = true;
        _saveAction(record);
    };

    for (auto& kv : m_position) {
        settle_holding(kv.second, true);
    }
    for (auto& kv : m_short_position) {
        settle_holding(kv.second, false);
    }

    // The maintenance requirement covers every margin-locked holding, the ones the posting above
    // skipped included (a bar-less instrument or one past its last registered bar): the risk check
    // has to be measured on the whole book, exactly like the equity below, otherwise a skipped
    // holding would count on the asset side without counting on the requirement side
    auto add_maintain = [&](const PositionRecord& position, bool is_long) {
        HKU_IF_RETURN(!(position.number > 0.0) || !(position.margin > 0.0), void());
        const Stock& stock = position.stock;
        price_t price = stock.getMarketValue(datetime, ktype);
        if (!(price > 0.0)) {
            // A holding without any quote is valued on its own basis, so that it cannot silently
            // drop out of the requirement
            double quantity = position.number * stock.unit();
            HKU_IF_RETURN(!(quantity > 0.0), void());
            price = positionSettleBasis(position, position.number) / quantity;
        }
        maintain_required = roundEx(
          maintain_required + getMaintainMargin(datetime, stock, price, position.number,
                                                is_long ? MarginSide::LONG : MarginSide::SHORT),
          precision);
    };
    for (const auto& kv : m_position) {
        add_maintain(kv.second, true);
    }
    for (const auto& kv : m_short_position) {
        add_maintain(kv.second, false);
    }

    // Daily risk check: the maintenance requirement of the settled holdings against the equity,
    // which the mark-to-market posting above has just brought onto the settlement price. The
    // equity is measured exactly like getFunds does, only at the settlement price instead of the
    // last trade moment (a settled day without a trade still has to be valued at its own close)
    price_t equity = m_cash;
    auto add_equity = [&](const PositionRecord& position, bool is_long) {
        HKU_IF_RETURN(!(position.number > 0.0), void());
        price_t price = position.stock.getMarketValue(datetime, ktype);
        price_t contribution = position.number * price * position.stock.unit();
        if (position.margin > 0.0) {
            equity = roundEx(equity + marginPositionEquity(position, price, is_long), precision);
        } else {
            equity = roundEx(equity + (is_long ? contribution : -contribution), precision);
        }
    };
    for (const auto& kv : m_position) {
        add_equity(kv.second, true);
    }
    for (const auto& kv : m_short_position) {
        add_equity(kv.second, false);
    }
    equity = roundEx(equity - m_borrow_cash, precision);

    bool called = false;
    if (maintain_required > 0.0) {
        called = equity < maintain_required;
        HKU_WARN_IF(called && !m_tmp_margin_called,
                    "{} {} Margin call! equity: {:<.4f}, maintenance requirement: {:<.4f}",
                    datetime, name(), equity, maintain_required);
        HKU_INFO_IF(!called && m_tmp_margin_called,
                    "{} {} Margin call cleared, equity: {:<.4f}, maintenance requirement: "
                    "{:<.4f}",
                    datetime, name(), equity, maintain_required);
    } else {
        HKU_INFO_IF(m_tmp_margin_called, "{} {} Margin call cleared, no margin holding left",
                    datetime, name());
    }
    m_tmp_margin_called = called;

    // The queue is rebuilt from the fresh equity at every settlement: a recovered account clears
    // it and a still short one arms it for the executor of the trading system
    bool was_queued = !m_tmp_force_liquidate_date.isNull();
    bool queue = called && tryGetParam<bool>("force_liquidate", false);
    m_tmp_force_liquidate_date = queue ? datetime : Null<Datetime>();
    HKU_WARN_IF(queue && !was_queued,
                "{} {} Margin call, the account will be liquidated at the next open!", datetime,
                name());
    // Nothing to post means nothing was settled: an empty account (or one whose whole book was
    // skipped) must not report itself as settled for the moment
    return posted;
}

void TradeManager::_saveAction(const TradeRecord& record) {
    HKU_IF_RETURN(getParam<bool>("save_action") == false, void());
    std::stringstream buf(std::stringstream::out);
    string my_tm("td = my_tm.");
    string sep(", ");
    // Escape characters that would break the regenerated script
    auto esc_remark = [](const string& s) {
        string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(c);
            }
        }
        return out;
    };
    switch (record.business) {
        case BUSINESS_INIT:
            buf << "my_tm = crtTM(datetime=Datetime('" << record.datetime.str() << "'), "
                << "initCash=" << record.cash << sep << "costFunc=";
            if (m_costfunc) {
                buf << m_costfunc->name() << "(" << m_costfunc->getParameter().getNameValueList()
                    << "), " << "name='" << m_name << "'" << ")";
            } else {
                buf << "TC_Zero()";
            }
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
                << sep << record.from << sep << "\"" << esc_remark(record.remark) << "\")";
            break;

        case BUSINESS_SELL:
            buf << my_tm << "sell(Datetime('" << record.datetime.str() << "')," << "sm['"
                << record.stock.market_code() << "'], " << record.realPrice << sep << record.number
                << sep << record.stoploss << sep << record.goalPrice << sep << record.planPrice
                << sep << record.from << sep << "\"" << esc_remark(record.remark) << "\")";
            break;

        case BUSINESS_SETTLEMENT:
            // Deliberately not replayed: the settlement posts a cash flow derived from the price
            // data and the margin model of that moment, so the regenerated script has to rebuild
            // it through settlement() rather than pin the numbers as a trade; the full state is
            // restored by replaying the trade records instead
            return;

        default:
            return;
    }

    m_actions.push_back(buf.str());
}

void TradeManager::tocsv(const string& path) {
    string date = m_init_datetime.str();
    // Sanitize the account name so it cannot escape the target directory
    auto safe = [](const string& s) {
        string out;
        out.reserve(s.size());
        for (unsigned char c : s) {
            if (c < 0x20 || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' ||
                c == '<' || c == '>' || c == '|' || c == '.') {
                out.push_back('_');
            } else {
                out.push_back(c);
            }
        }
        return out;
    };
    string prefix = m_name.empty() ? date : safe(m_name);
    string filename1 = path + "/" + prefix + "_" + lang_htr("trade_records") + ".csv";
    string filename2 = path + "/" + prefix + "_" + lang_htr("closed_positions") + ".csv";
    string filename3 = path + "/" + prefix + "_" + lang_htr("open_positions") + ".csv";
    string filename4 = path + "/" + prefix + "_actions.txt";
    string filename5 = path + "/" + prefix + "_" + lang_htr("short_positions") + ".csv";
    string filename6 = path + "/" + prefix + "_" + lang_htr("borrow_stock") + ".csv";

#if defined(_MSC_VER)
    filename1 = utf8_to_gb(filename1);
    filename2 = utf8_to_gb(filename2);
    filename3 = utf8_to_gb(filename3);
    filename4 = utf8_to_gb(filename4);
    filename5 = utf8_to_gb(filename5);
    filename6 = utf8_to_gb(filename6);
#endif

    string sep(",");

    // Export the trade records
    std::ofstream file(filename1.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename1);

    // The key itself is used as the column name when no translation is available
    auto col = [](const std::string& name) { return name + ","; };

    file.setf(std::ios_base::fixed);
    file.precision(3);
    file << "#" << col(htr("fill_date")) << col(htr("market_code")) << col(htr("stock_name"))
         << col(htr("business")) << col(htr("planPrice")) << col(htr("realPrice"))
         << col(htr("goalPrice")) << col(htr("fill_number")) << col(htr("commission"))
         << col(htr("stamp_tax")) << col(htr("transfer_fee")) << col(htr("other_cost"))
         << col(htr("cost_total")) << col(htr("stoploss")) << col(htr("cash"))
         << col(htr("signal_part")) << col(htr("Date")) << col(htr("Open")) << col(htr("High"))
         << col(htr("Low")) << col(htr("Close")) << col(htr("Amount")) << col(htr("Volume"))
         << col(htr("remark")) << std::endl;
    TradeRecordList::const_iterator trade_iter = m_trade_list.begin();
    for (; trade_iter != m_trade_list.end(); ++trade_iter) {
        const TradeRecord& record = *trade_iter;
        if (record.stock.isNull()) {
            file << record.datetime << sep << sep << sep << getBusinessName(record.business) << sep
                 << record.planPrice << sep << record.realPrice << sep << record.goalPrice << sep
                 << record.number << sep << record.cost.commission << sep << record.cost.stamptax
                 << sep << record.cost.transferfee << sep << record.cost.others << sep
                 << record.cost.total << sep << record.stoploss << sep << record.cash << sep
                 << getSystemPartName(record.from) << sep << sep << sep << sep << sep << sep << sep
                 << sep << record.remark << std::endl;
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
                         << kdata.transAmount << sep << kdata.transCount << sep << record.remark;
                } else {
                    file << sep << sep << sep << sep << sep << sep << sep << sep << record.remark;
                }
            } else {
                file << sep << sep << sep << sep << sep << sep << sep << record.remark;
            }
            file << std::endl;
        }
    }
    file.close();

    // Export the closed position records
    file.open(filename2.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename2);
    file << "#" << col(htr("entry_date")) << col(htr("exit_date")) << col(htr("market_code"))
         << col(htr("stock_name")) << col(htr("total_number")) << col(htr("buy_money"))
         << col(htr("total_cost")) << col(htr("sell_money")) << col(htr("total_profit"))
         << col(htr("total_risk")) << col(htr("profit_loss_ratio")) << col(htr("hold_days"))
         << col(htr("buy_count")) << col(htr("sell_count")) << std::endl;
    PositionRecordList::const_iterator history_iter = m_position_history.begin();
    for (; history_iter != m_position_history.end(); ++history_iter) {
        const PositionRecord& record = *history_iter;
        file << record.takeDatetime << sep << record.cleanDatetime << sep
             << record.stock.market_code() << sep << record.stock.name() << sep
             << record.totalNumber << sep << record.buyMoney << sep << record.totalCost << sep
             << record.sellMoney << sep << record.sellMoney - record.totalCost - record.buyMoney
             << sep << record.totalRisk << sep
             << (record.buyMoney + record.totalCost != 0.0
                   ? record.totalProfit() / (record.buyMoney + record.totalCost)
                   : 0.0)
             << sep << (record.cleanDatetime - record.takeDatetime).days() << sep << record.buyCount
             << sep << record.sellCount << std::endl;
    }
    file.close();

    // Export the open position records
    file.open(filename3.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename3);
    file << "#" << col(htr("entry_date")) << col(htr("exit_date")) << col(htr("market_code"))
         << col(htr("stock_name")) << col(htr("current_number")) << col(htr("total_number"))
         << col(htr("buy_money")) << col(htr("total_cost")) << col(htr("sell_money"))
         << col(htr("total_risk")) << col(htr("buy_count")) << col(htr("sell_count"))
         << col(htr("floating_profit")) << col(htr("profit_cost_price"))
         << col(htr("floating_profit_ratio")) << std::endl;
    position_map_type::const_iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        const PositionRecord& record = position_iter->second;
        file << record.takeDatetime << sep << record.cleanDatetime << sep
             << record.stock.market_code() << sep << record.stock.name() << sep << record.number
             << sep << record.totalNumber << sep << record.buyMoney << sep << record.totalCost
             << sep << record.sellMoney << sep << record.totalRisk << sep << record.buyCount << sep
             << record.sellCount << sep;
        size_t pos = record.stock.getCount(KQuery::DAY);
        if (pos != 0) {
            KRecord krecord = record.stock.getKRecord(pos - 1, KQuery::DAY);
            price_t bonus = record.buyMoney - record.sellMoney + record.totalCost;
            auto sellCost =
              getSellCost(krecord.datetime, record.stock, krecord.closePrice, record.number);
            price_t profit = record.number * krecord.closePrice + record.sellMoney -
                             record.buyMoney - record.totalCost - sellCost.total;
            price_t cost_base = record.buyMoney + record.totalCost + sellCost.total;
            file << record.number * krecord.closePrice - bonus << sep
                 << (record.number != 0.0 ? bonus / record.number : 0.0) << sep
                 << (cost_base != 0.0 ? profit / cost_base : 0.0) << std::endl;
        } else {
            // No day data yet: still terminate the row so the next record does not concatenate
            file << sep << sep << std::endl;
        }
    }
    file.close();

    // Export the order execution commands
    // Export the closed position records
    file.open(filename4.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename4);
    list<string>::const_iterator action_iter = m_actions.begin();
    for (; action_iter != m_actions.end(); ++action_iter) {
        file << *action_iter << std::endl;
    }
    file.close();

    // Export the short positions (open + closed)
    file.open(filename5.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename5);
    file << "#" << col(htr("kind")) << col(htr("entry_date")) << col(htr("exit_date"))
         << col(htr("market_code")) << col(htr("stock_name")) << col(htr("number"))
         << col(htr("total_number")) << col(htr("buy_money")) << col(htr("total_cost"))
         << col(htr("sell_money")) << col(htr("total_risk")) << std::endl;
    auto writeShort = [&](const char* kind, const PositionRecord& r) {
        file << kind << sep << r.takeDatetime << sep << r.cleanDatetime << sep
             << r.stock.market_code() << sep << r.stock.name() << sep << r.number << sep
             << r.totalNumber << sep << r.buyMoney << sep << r.totalCost << sep << r.sellMoney
             << sep << r.totalRisk << std::endl;
    };
    for (const auto& kv : m_short_position)
        writeShort("open", kv.second);
    for (const auto& r : m_short_position_history)
        writeShort("closed", r);
    file.close();

    // Export the current borrow stock
    file.open(filename6.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename6);
    file << "#" << col(htr("market_code")) << col(htr("stock_name")) << col(htr("number"))
         << col(htr("value")) << std::endl;
    for (const auto& kv : m_borrow_stock) {
        const BorrowRecord& r = kv.second;
        file << r.stock.market_code() << sep << r.stock.name() << sep << r.number << sep << r.value
             << std::endl;
    }
    file.close();
}

bool TradeManager::addPosition(const PositionRecord& pr) {
    HKU_ERROR_IF_RETURN(pr.stock.isNull(), false, "Invalid position record! stock is null!");
    HKU_ERROR_IF_RETURN(pr.cleanDatetime != Null<Datetime>(), false,
                        "Position cleanDatetime({}) must be Null!", pr.cleanDatetime);
    HKU_ERROR_IF_RETURN(pr.takeDatetime < initDatetime(), false,
                        "Position takeDatetime({}) must be >= initDatetime({})!", pr.takeDatetime,
                        initDatetime());
    HKU_ERROR_IF_RETURN(m_trade_list.size() > 1, false, "Exist trade list!");

    auto iter = m_position.find(pr.stock.id());
    HKU_ERROR_IF_RETURN(iter != m_position.end(), false, "The stock({}) has position!",
                        pr.stock.market_code());

    m_position[pr.stock.id()] = pr;
    // A freshly injected holding has never been scanned; drop any stale frontier for this stock
    m_tmp_weight_checked_until.erase(pr.stock.id());
    if (pr.takeDatetime > m_init_datetime) {
        m_init_datetime = pr.takeDatetime;
        m_trade_list[0].datetime = m_init_datetime;
    }
    return true;
}

bool TradeManager::addTradeRecord(const TradeRecord& tr) {
    HKU_IF_RETURN(BUSINESS_INIT == tr.business, _add_init_tr(tr));
    HKU_ERROR_IF_RETURN(tr.datetime < lastDatetime(), false,
                        "tr.datetime must be >= lastDatetime({})!", lastDatetime());
    // An invalid record must not advance the account (updateWithWeight) before it is rejected
    HKU_ERROR_IF_RETURN(BUSINESS_INVALID == tr.business, false, "tr.business is invalid!");

    updateWithWeight(tr.datetime);

    // A backfilled record may increase a holding earlier than an already advanced check frontier;
    // drop the frontier so the added shares get rescanned (mirrors the live trade entries)
    if (!tr.stock.isNull() &&
        (BUSINESS_BUY == tr.business || BUSINESS_CHECKIN_STOCK == tr.business ||
         BUSINESS_BORROW_STOCK == tr.business || BUSINESS_SELL_SHORT == tr.business)) {
        m_tmp_weight_checked_until.erase(tr.stock.id());
    }

    switch (tr.business) {
        case BUSINESS_INIT:
            return false;

        case BUSINESS_BUY:
            return _add_buy_tr(tr);

        case BUSINESS_SELL:
            return _add_sell_tr(tr);

        // The ex-rights derived records (bonus share / consolidation / dividend / compensation and
        // the short/borrow quantity adjustments) are regenerated by updateWithWeight as the
        // primary records advance the account, so replaying them directly is a no-op
        case BUSINESS_GIFT:
        case BUSINESS_SUOGU:
        case BUSINESS_BORROW_ADJUST:
        case BUSINESS_SHORT_ADJUST:
            return true;

        case BUSINESS_BONUS:
        case BUSINESS_DIVIDEND_COMPENSATION:
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

        case BUSINESS_SETTLEMENT:
            return _add_settlement_tr(tr);

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
    // Mirror the live buy: a margin-accounted opening locks only the margin; a model reporting no
    // requirement at all leaves the record on the cash books. The record carries the amount it
    // locked, so the backfill reproduces the books exactly; an archive written before that field
    // existed falls back to the model
    bool margin_acct = isMarginAccounted(tr.stock) && tr.realPrice > 0.0;
    price_t locked_margin =
      margin_acct ? (tr.margin > 0.0 ? tr.margin
                                     : getOpenMarginToLock(tr.datetime, tr.stock, tr.realPrice,
                                                           tr.number, MarginSide::LONG))
                  : 0.0;
    HKU_WARN_IF(margin_acct && !(locked_margin > 0.0),
                "{} {} The margin model reported no requirement, the opening is booked by cash!",
                tr.datetime, tr.stock.market_code());
    margin_acct = margin_acct && locked_margin > 0.0;
    price_t need = margin_acct ? locked_margin : money;

    // The record carries the trade only: when the live books financed the shortfall, the add path
    // reproduces the debt instead of rejecting a trade the account really filled
    price_t need_cash = roundEx(need + tr.cost.total, precision);
    if (m_cash < need_cash) {
        HKU_WARN_IF_RETURN(!_borrowForAdd(tr.datetime, need_cash), false,
                           "Don't have enough money! {} < {}, {}", m_cash, need_cash, tr);
    }

    m_cash = roundEx(m_cash - need - tr.cost.total, precision);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    // Update the current position record
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord position(
          tr.stock, tr.datetime, Null<Datetime>(), tr.number, tr.stoploss, tr.goalPrice, tr.number,
          money, tr.cost.total,
          roundEx((tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(), precision), 0.0);
        position.buyCount = 1;
        if (margin_acct) {
            position.margin = need;
        }
        m_position[tr.stock.id()] = position;
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += tr.number;
        position.stoploss = tr.stoploss;
        position.goalPrice = tr.goalPrice;
        position.totalNumber += tr.number;
        position.buyMoney = roundEx(money + position.buyMoney, precision);
        position.openBasis = roundEx(position.openBasis + money, precision);
        position.settleBasis = roundEx(position.settleBasis + money, precision);
        position.totalCost = roundEx(tr.cost.total + position.totalCost, precision);
        position.totalRisk =
          roundEx(position.totalRisk + (tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(),
                  precision);
        if (margin_acct) {
            // Margin lots joining a cash-basis holding mix the two accountings in one record
            HKU_WARN_IF(position.margin == 0.0,
                        "{} Margin lots join a cash-basis holding, the books mix the two "
                        "accountings!",
                        tr.datetime);
            position.margin = roundEx(position.margin + need, precision);
        } else if (position.margin > 0.0) {
            // The other mixing direction: a cash-basis lot joins margin lots
            HKU_WARN_IF(true,
                        "{} Cash lots join a margin-accounted holding, the books mix the two "
                        "accountings!",
                        tr.datetime);
        }
        position.buyCount++;
    }

    _saveAction(new_tr);

    return true;
}

bool TradeManager::_add_sell_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is Null!");
    HKU_ERROR_IF_RETURN(tr.number == 0, false, "tr.number is zero!");

    // There is no position
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "No position!");

    PositionRecord& position = pos_iter->second;

    // The quantity to be sold is greater than the current position quantity
    HKU_ERROR_IF_RETURN(position.number < tr.number, false, "Try sell number greater position!");

    int precision = getParam<int>("precision");
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    // Update the cash balance; a margin-accounted holding returns the released margin and the
    // realized profit only (mirrors the live sell). A realPrice 0 placeholder cannot be valued and
    // would silently drop the margin, so the record is rejected
    HKU_WARN_IF_RETURN(position.margin > 0.0 && tr.realPrice <= 0.0, false,
                       "A margin-accounted holding can't be closed at a placeholder price(0)!");
    price_t released = 0.0;
    // The cost notional and the settlement basis of the lots being closed, both released from the
    // books proportionally
    price_t basis = positionOpenBasis(position, tr.number);
    price_t mtmBasis = positionSettleBasis(position, tr.number);
    if (position.margin > 0.0) {
        released = roundEx(position.margin * (tr.number / position.number), precision);
        price_t realized = roundEx(money - mtmBasis, precision);
        m_cash = roundEx(m_cash + released + realized - tr.cost.total, precision);
    } else {
        m_cash = roundEx(m_cash + money - tr.cost.total, precision);
    }

    // Update the trade records
    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    // Update the current position
    position.number -= tr.number;
    position.margin = position.number > 0.0 ? roundEx(position.margin - released, precision) : 0.0;
    position.openBasis =
      position.number > 0.0 ? roundEx(position.openBasis - basis, precision) : 0.0;
    position.settleBasis =
      position.number > 0.0 ? roundEx(position.settleBasis - mtmBasis, precision) : 0.0;
    position.stoploss = tr.stoploss;
    position.goalPrice = tr.goalPrice;
    // position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + tr.cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);
    position.sellCount++;

    if (position.number == 0) {
        position.cleanDatetime = tr.datetime;
        m_position_history.push_back(position);
        // Delete the current position
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
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    int precision = getParam<int>("precision");
    price_t market_value = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord pos(tr.stock, tr.datetime, Null<Datetime>(), tr.number, 0.0, 0.0, tr.number,
                           market_value, 0.0, 0.0, 0.0);
        pos.buyCount = 1;
        m_position[tr.stock.id()] = pos;
    } else {
        PositionRecord& pos = pos_iter->second;
        // Same rejection as the live checkinStock: the deposit can't join a margin-accounted
        // holding
        HKU_ERROR_IF_RETURN(pos.margin > 0.0, false,
                            "{} {} Can't deposit into a margin-accounted holding, its margin books "
                            "are not supported yet!",
                            tr.datetime, tr.stock.market_code());
        pos.number += tr.number;
        pos.totalNumber += tr.number;
        pos.buyMoney = roundEx(pos.buyMoney + market_value, precision);
        pos.openBasis = roundEx(pos.openBasis + market_value, precision);
        pos.settleBasis = roundEx(pos.settleBasis + market_value, precision);
        pos.buyCount++;
    }

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    m_checkin_stock = roundEx(m_checkin_stock + market_value, precision);
    return true;
}

bool TradeManager::_add_checkout_stock_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "Try to checkout nonexistent stock!");

    int precision = getParam<int>("precision");
    PositionRecord& pos = pos_iter->second;
    HKU_ERROR_IF_RETURN(tr.number > pos.number, false, "Try to checkout beyond position number!");

    price_t market_value = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);
    // Same rejection as the live checkoutStock: the withdrawal can't leave the margin books
    HKU_ERROR_IF_RETURN(pos.margin > 0.0, false,
                        "{} {} Can't withdraw from a margin-accounted holding, its margin books "
                        "are not supported yet!",
                        tr.datetime, tr.stock.market_code());
    // Withdrawing lots releases their cost basis proportionally (the price only feeds the
    // reporting sellMoney, the cost pool follows the quantity)
    price_t basis = positionOpenBasis(pos, tr.number);
    price_t mtmBasis = positionSettleBasis(pos, tr.number);
    pos.number -= tr.number;
    pos.openBasis = pos.number > 0.0 ? roundEx(pos.openBasis - basis, precision) : 0.0;
    pos.settleBasis = pos.number > 0.0 ? roundEx(pos.settleBasis - mtmBasis, precision) : 0.0;
    pos.sellMoney = roundEx(pos.sellMoney + market_value, precision);
    pos.sellCount++;
    if (0 == pos.number) {
        pos.cleanDatetime = tr.datetime;
        m_position_history.push_back(pos);
        m_position.erase(tr.stock.id());
    }

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    m_checkout_stock = roundEx(m_checkout_stock + market_value, precision);
    return true;
}

bool TradeManager::_add_borrow_cash_tr(const TradeRecord& tr) {
    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(tr.realPrice, precision);
    HKU_ERROR_IF_RETURN(in_cash <= 0.0, false, "tr.realPrice({:<.4f}) must be > 0!", tr.realPrice);

    m_cash = roundEx(m_cash + in_cash - tr.cost.total, precision);
    m_borrow_cash = roundEx(m_borrow_cash + in_cash, precision);
    m_loan_list.push_back(LoanRecord(tr.datetime, in_cash));

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_return_cash_tr(const TradeRecord& tr) {
    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(tr.realPrice, precision);
    HKU_ERROR_IF_RETURN(in_cash <= 0.0, false, "tr.realPrice({:<.4f}) must be > 0!", tr.realPrice);
    HKU_ERROR_IF_RETURN(m_loan_list.empty(), false, "No borrow cash to return!");

    price_t out_cash = roundEx(in_cash + tr.cost.total, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false, "Return cash({:<.4f}) > current cash({:<.4f})!",
                        out_cash, m_cash);

    // Reject repaying more than the outstanding debt before draining the loan list (matching the
    // live returnCash: validate first, then mutate; grid residual treated as fully repaid)
    price_t epsilon = 0.5 * std::pow(10.0, -double(precision));
    price_t total_loan = 0.0;
    for (const auto& loan : m_loan_list) {
        total_loan = roundEx(total_loan + loan.value, precision);
    }
    HKU_ERROR_IF_RETURN(in_cash - total_loan > epsilon, false,
                        "{} return cash({:<.4f}) > borrowed cash({:<.4f})!", tr.datetime, in_cash,
                        total_loan);

    // Consume the loan list in FIFO order using the record's principal
    price_t remain = in_cash;
    do {
        list<LoanRecord>::iterator iter = m_loan_list.begin();
        if (remain >= iter->value - epsilon) {
            remain = roundEx(remain - iter->value, precision);
            m_loan_list.pop_front();
        } else {
            iter->value = roundEx(iter->value - remain, precision);
            break;
        }
    } while (!m_loan_list.empty() && remain > epsilon);

    m_cash = roundEx(m_cash - out_cash, precision);
    m_borrow_cash = roundEx(m_borrow_cash - in_cash, precision);

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_borrow_stock_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    int precision = getParam<int>("precision");
    price_t market_value = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);
    // The lending fee is paid out of the cash: the live short opening finances it together with the
    // margin, so the add path reproduces that debt instead of driving the cash negative
    if (getParam<bool>("support_borrow_cash") && m_cash < tr.cost.total) {
        HKU_WARN_IF_RETURN(!_borrowForAdd(tr.datetime, tr.cost.total), false,
                           "{} Can't finance the borrow-stock fee({:<.4f})!", tr.datetime,
                           tr.cost.total);
    }
    m_cash = roundEx(m_cash - tr.cost.total, precision);

    borrow_stock_map_type::iterator iter = m_borrow_stock.find(tr.stock.id());
    if (iter == m_borrow_stock.end()) {
        BorrowRecord record(tr.stock, tr.number, market_value);
        record.record_list.push_back(BorrowRecord::Data(tr.datetime, tr.realPrice, tr.number));
        m_borrow_stock[tr.stock.id()] = record;
    } else {
        iter->second.number += tr.number;
        iter->second.value = roundEx(iter->second.value + market_value, precision);
        iter->second.record_list.push_back(
          BorrowRecord::Data(tr.datetime, tr.realPrice, tr.number));
    }

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_return_stock_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    borrow_stock_map_type::iterator bor_iter = m_borrow_stock.find(tr.stock.id());
    HKU_ERROR_IF_RETURN(bor_iter == m_borrow_stock.end(), false,
                        "Try to return nonborrowed stock!");

    int precision = getParam<int>("precision");
    BorrowRecord& bor = bor_iter->second;
    HKU_ERROR_IF_RETURN(tr.number > bor.number, false, "Try to return beyond borrow number!");

    // The returned market value comes from the consumed entries' borrow prices (same as the live
    // returnStock path)
    price_t market_value = 0.0;
    double remain = tr.number;
    list<BorrowRecord::Data>::iterator iter = bor.record_list.begin();
    for (; iter != bor.record_list.end(); ++iter) {
        double take = (remain <= iter->number) ? remain : iter->number;
        market_value = roundEx(market_value + iter->price * take * tr.stock.unit(), precision);
        remain -= take;
        if (remain <= 0.0) {
            break;
        }
    }

    remain = tr.number;
    do {
        iter = bor.record_list.begin();
        if (remain >= iter->number) {
            remain = roundEx(remain - iter->number, precision);
            bor.record_list.pop_front();
        } else {
            iter->number = roundEx(iter->number - remain, precision);
            break;
        }
    } while (!bor.record_list.empty() && remain > 0.0);

    bor.number -= tr.number;
    bor.value = roundEx(bor.value - market_value, precision);
    if (bor.record_list.empty()) {
        m_borrow_stock.erase(bor_iter);
    }

    m_cash = roundEx(m_cash - tr.cost.total, precision);
    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);
    return true;
}

bool TradeManager::_add_sell_short_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    int precision = getParam<int>("precision");
    // net proceeds, mirroring the live sellShort cash formula
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit() - tr.cost.total, precision);
    // Mirror the live sellShort: a margin-accounted short opening locks only the margin and the
    // sale proceeds never enter the cash. The record carries the amount it locked, so the backfill
    // reproduces the books exactly; an archive written before that field existed falls back to the
    // model
    bool margin_acct = isMarginAccounted(tr.stock) && tr.realPrice > 0.0;
    price_t need = margin_acct
                     ? (tr.margin > 0.0 ? tr.margin
                                        : getOpenMarginToLock(tr.datetime, tr.stock, tr.realPrice,
                                                              tr.number, MarginSide::SHORT))
                     : 0.0;
    HKU_WARN_IF(margin_acct && !(need > 0.0),
                "{} {} The margin model reported no requirement, the short opening is booked by "
                "cash!",
                tr.datetime, tr.stock.market_code());
    margin_acct = margin_acct && need > 0.0;
    if (margin_acct) {
        // Same funding rule as the replayed long opening: the live books may have financed it
        price_t need_cash = roundEx(need + tr.cost.total, precision);
        if (m_cash < need_cash) {
            HKU_WARN_IF_RETURN(!_borrowForAdd(tr.datetime, need_cash), false,
                               "Don't have enough money! {} < {}, {}", m_cash, need_cash, tr);
        }
    }
    m_cash = margin_acct ? roundEx(m_cash - need - tr.cost.total, precision)
                         : roundEx(m_cash + money, precision);
    // Gross proceeds on the position; fees only in totalCost (mirrors the live sellShort)
    price_t gross = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    // The record states the locked amount, so an archive without it is completed here
    new_tr.margin = margin_acct ? need : 0.0;
    m_trade_list.push_back(new_tr);

    price_t risk =
      tr.stoploss > 0.0
        ? roundEx((tr.stoploss - tr.realPrice) * tr.number * tr.stock.unit(), precision)
        : 0.0;
    position_map_type::iterator pos_iter = m_short_position.find(tr.stock.id());
    if (pos_iter == m_short_position.end()) {
        PositionRecord position(tr.stock, tr.datetime, Null<Datetime>(), tr.number, tr.stoploss,
                                tr.goalPrice, tr.number, 0.0, tr.cost.total, risk, gross);
        position.sellCount = 1;
        // The gross opening proceeds of a short are its cost basis and its settlement basis (the
        // long side uses buyMoney)
        position.openBasis = gross;
        position.settleBasis = gross;
        if (margin_acct) {
            position.margin = need;
        }
        m_short_position[tr.stock.id()] = position;
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += tr.number;
        position.stoploss = tr.stoploss;
        position.goalPrice = tr.goalPrice;
        position.totalNumber += tr.number;
        position.totalCost = roundEx(tr.cost.total + position.totalCost, precision);
        position.totalRisk = roundEx(position.totalRisk + risk, precision);
        position.sellMoney = roundEx(position.sellMoney + gross, precision);
        position.openBasis = roundEx(position.openBasis + gross, precision);
        position.settleBasis = roundEx(position.settleBasis + gross, precision);
        if (margin_acct) {
            // Margin lots joining a cash-basis holding mix the two accountings in one record
            HKU_WARN_IF(position.margin == 0.0,
                        "{} Margin lots join a cash-basis holding, the books mix the two "
                        "accountings!",
                        tr.datetime);
            position.margin = roundEx(position.margin + need, precision);
        } else if (position.margin > 0.0) {
            // The other mixing direction: a cash-basis lot joins margin lots
            HKU_WARN_IF(true,
                        "{} Cash lots join a margin-accounted short, the books mix the two "
                        "accountings!",
                        tr.datetime);
        }
        position.sellCount++;
    }

    _saveAction(new_tr);
    return true;
}

bool TradeManager::_add_buy_short_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(!std::isfinite(tr.number) || tr.number <= 0, false,
                        "tr.number({}) must be finite and > 0!", tr.number);

    position_map_type::iterator pos_iter = m_short_position.find(tr.stock.id());
    HKU_WARN_IF_RETURN(pos_iter == m_short_position.end(), false, "No short position to cover!");
    PositionRecord& position = pos_iter->second;

    // The record's number is the executed cover quantity (the live path already resolved
    // MAX_DOUBLE / clamp), and the separate RETURN_STOCK record handles the borrow pool
    HKU_ERROR_IF_RETURN(tr.number > position.number, false, "Cover number beyond short position!");

    int precision = getParam<int>("precision");
    price_t money = roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);
    // Mirror the live buyShort: a margin-accounted short returns the released margin and the
    // realized profit instead of paying the notional, an overdraft close allowed like the long
    // one; a realPrice 0 placeholder would silently drop the margin, so reject it
    HKU_WARN_IF_RETURN(position.margin > 0.0 && tr.realPrice <= 0.0, false,
                       "A margin-accounted holding can't be closed at a placeholder price(0)!");
    price_t released = 0.0;
    // The cost notional and the settlement basis of the lots being covered, both released from
    // the books proportionally
    price_t basis = positionOpenBasis(position, tr.number);
    price_t mtmBasis = positionSettleBasis(position, tr.number);
    if (position.margin > 0.0) {
        released = roundEx(position.margin * (tr.number / position.number), precision);
        price_t realized = roundEx(mtmBasis - money, precision);
        m_cash = roundEx(m_cash + released + realized - tr.cost.total, precision);
    } else {
        HKU_WARN_IF_RETURN(m_cash < roundEx(money + tr.cost.total, precision), false,
                           "Can't buyShort, need cash({:<.4f}) > current cash({:<.4f})!",
                           roundEx(money + tr.cost.total, precision), m_cash);
        m_cash = roundEx(m_cash - money - tr.cost.total, precision);
    }

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    double pre_number = position.number;
    position.number -= tr.number;
    if (position.number > 0.0 && pre_number > 0.0) {
        position.totalRisk =
          roundEx(position.totalRisk * (position.number / pre_number), precision);
    }
    position.margin = position.number > 0.0 ? roundEx(position.margin - released, precision) : 0.0;
    position.openBasis =
      position.number > 0.0 ? roundEx(position.openBasis - basis, precision) : 0.0;
    position.settleBasis =
      position.number > 0.0 ? roundEx(position.settleBasis - mtmBasis, precision) : 0.0;
    position.buyMoney = roundEx(position.buyMoney + money, precision);
    position.totalCost = roundEx(position.totalCost + tr.cost.total, precision);
    position.buyCount++;
    if (position.number == 0) {
        position.cleanDatetime = tr.datetime;
        m_short_position_history.push_back(position);
        m_short_position.erase(tr.stock.id());
    }

    _saveAction(new_tr);
    return true;
}

bool TradeManager::_add_settlement_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
    HKU_ERROR_IF_RETURN(tr.number == 0.0, false, "tr.number is zero!");
    HKU_ERROR_IF_RETURN(!(tr.realPrice > 0.0), false, "tr.realPrice <= 0.0!");

    // The sign of the record number is the settled side, see settlement()
    const bool is_long = tr.number > 0.0;
    // Idempotent: the posting is a real cash flow, so the same record applied twice would book the
    // profit twice. The settlement is per holding, so the probe must match the stock and the side
    bool applied = false;
    for (auto iter = m_trade_list.rbegin(); iter != m_trade_list.rend(); ++iter) {
        if (iter->datetime < tr.datetime) {
            break;
        }
        if (iter->business == BUSINESS_SETTLEMENT && iter->datetime == tr.datetime &&
            iter->stock.id() == tr.stock.id() && (iter->number > 0.0) == is_long) {
            applied = true;
            break;
        }
    }
    HKU_WARN_IF_RETURN(applied, true, "{} {}: the settlement has been applied already!",
                       tr.datetime, tr.stock.market_code());

    position_map_type& positions = is_long ? m_position : m_short_position;
    position_map_type::iterator pos_iter = positions.find(tr.stock.id());
    HKU_WARN_IF_RETURN(pos_iter == positions.end(), false, "{} {} No holding to settle!",
                       tr.datetime, tr.stock.market_code());
    PositionRecord& position = pos_iter->second;

    int precision = getParam<int>("precision");
    // The record number only carries the side, the state it restores is the whole holding's, so the
    // current quantity is what the mark-to-market amount is computed on
    HKU_WARN_IF(std::abs(std::abs(tr.number) - position.number) > 1e-6,
                "{} {}: the settled quantity({}) differs from the holding({}), the holding is "
                "used!",
                tr.datetime, tr.stock.market_code(), std::abs(tr.number), position.number);
    price_t mv = roundEx(tr.realPrice * position.number * tr.stock.unit(), precision);
    m_cash = roundEx(m_cash + (is_long ? mv - position.settleBasis : position.settleBasis - mv),
                     precision);
    position.settleBasis = mv;
    // The record always carries the lock snapshot (the field was introduced together with this
    // business type), so a zero means the model relocked nothing and the holding moves to the cash
    // books
    price_t relock = roundEx(std::max<price_t>(tr.margin, 0.0), precision);
    // The margin difference is posted like the live settlement: the lock sits on the asset side, so
    // the released part must return to the cash, otherwise the equity would absorb the whole move
    price_t delta = roundEx(relock - position.margin, precision);
    if (delta > 0.0 && m_cash < delta) {
        HKU_WARN_IF(!_borrowForAdd(tr.datetime, delta),
                    "{} {}: can't finance the margin increase({:<.4f}) of the settlement, the "
                    "account goes overdrawn!",
                    tr.datetime, tr.stock.market_code(), delta);
    }
    m_cash = roundEx(m_cash - delta, precision);
    position.margin = relock;

    TradeRecord new_tr(tr);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    _saveAction(new_tr);
    return true;
}

} /* namespace hku */
