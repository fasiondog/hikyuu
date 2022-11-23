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
#include "../utilities/util.h"
#include "../utilities/thread/StealThreadPool.h"
#include "../KData.h"

namespace hku {

string TradeManager::str() const {
    std::stringstream os;
    os << std::fixed;
    os.precision(2);

    FundsRecord funds = _getFunds();
    string strip(",\n");
    os << "TradeManager {\n"
       << "  params: " << getParameter() << strip << "  name: " << name() << strip
       << "  init_date: " << initDatetime() << strip << "  init_cash: " << initCash() << strip
       << "  firstDatetime: " << firstDatetime() << strip << "  lastDatetime: " << lastDatetime()
       << strip << "  TradeCostFunc: " << costFunc() << strip
       << "  MarginRatioFunc: " << marginRatioFunc() << strip << "  current cash: " << currentCash()
       << strip;
    if (getParam<bool>("use_contract")) {
        os << "  current frozen cash: " << funds.market_value << strip;
    } else {
        os << "  current market_value: " << funds.market_value << strip;
    }
    os << "  current base_cash: " << funds.base_cash << strip
       << "  current base_asset: " << funds.base_asset << strip << "  Position: \n";

    StockManager& sm = StockManager::instance();
    KQuery query(-1);
    PositionRecordList position = getPositionList();
    PositionRecordList::const_iterator iter = position.begin();
    for (; iter != position.end(); ++iter) {
        price_t invest = iter->buyMoney - iter->sellMoney + iter->totalCost;
        KData k = iter->stock.getKData(query);
        price_t cur_val = k[0].closePrice * iter->number;
        price_t bonus = iter->calculateCloseProfit(k[0].closePrice);
        DatetimeList date_list =
          sm.getTradingCalendar(KQueryByDate(Datetime(iter->takeDatetime.date())));
        os << "    " << iter->stock.market_code() << " " << iter->stock.name() << " "
           << iter->takeDatetime << " " << date_list.size() << " " << iter->number << " " << invest
           << " " << cur_val << " " << bonus << " " << 100 * bonus / invest << "% "
           << 100 * bonus / m_init_cash << "%\n";
    }

    os << "}";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

TradeManager::TradeManager(const Datetime& datetime, price_t initcash, const TradeCostPtr& costfunc,
                           const MarginRatioPtr& mrfunc, const string& name)
: TradeManagerBase(name, costfunc, mrfunc),
  m_init_datetime(datetime),
  m_last_update_datetime(datetime),
  m_checkout_cash(0.0),
  m_checkin_stock(0.0),
  m_checkout_stock(0.0) {
    setParam<bool>("use_contract", false);  // 是否合约交易
    setParam<bool>("save_action", true);    // 是否保存命令
    m_init_cash = roundEx(initcash, 2);
    m_cash = m_init_cash;
    m_checkin_cash = m_init_cash;
    m_trade_list.push_back(TradeRecord(Null<Stock>(), m_init_datetime, BUSINESS_INIT, m_init_cash,
                                       m_init_cash, 0.0, 0, CostRecord(), 0.0, m_cash, 1.0,
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

    m_trade_list.clear();
    m_trade_list.emplace_back(Null<Stock>(), m_init_datetime, BUSINESS_INIT, m_init_cash,
                              m_init_cash, 0.0, 0, CostRecord(), 0.0, m_cash, 1.0, PART_INVALID);

    m_position.clear();
    m_position_history.clear();
    m_actions.clear();
    _saveAction(m_trade_list.back());
}

TradeManagerPtr TradeManager::_clone() {
    TradeManager* p = new TradeManager(m_init_datetime, m_init_cash, m_costfunc, m_mrfunc, m_name);
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
    p->m_trade_list = m_trade_list;
    p->m_position = m_position;
    p->m_position_history = m_position_history;
    p->m_broker_list = m_broker_list;
    p->m_broker_last_datetime = m_broker_last_datetime;

    p->m_actions = m_actions;

    return TradeManagerPtr(p);
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
    //日期小于账户建立日期，返回0
    HKU_IF_RETURN(datetime < m_init_datetime, 0.0);

    //根据权息信息调整持仓数量
    updateWithWeight(datetime);

    //如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            return pos_iter->second.number;
        }
        return 0.0;
    }

    //在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    double number = 0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        //交易记录中的交易日期已经大于查询日期，则跳出循环
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business) {
                number -= iter->number;

            } else {
                //其他情况忽略
            }
        }
    }
    return number;
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

PositionRecord TradeManager::getPosition(const Datetime& datetime, const Stock& stock) {
    PositionRecord result;
    HKU_IF_RETURN(stock.isNull(), result);
    HKU_IF_RETURN(datetime < m_init_datetime, result);

    //根据权息信息调整持仓数量
    updateWithWeight(datetime);

    //如果指定的日期大于等于最后交易日期，则直接取当前持仓记录
    if (datetime >= lastDatetime()) {
        position_map_type::const_iterator pos_iter = m_position.find(stock.id());
        if (pos_iter != m_position.end()) {
            result = pos_iter->second;
        }
        return result;
    }

    //在历史交易记录中，重新计算在指定的查询日期时，该交易对象的持仓数量
    double number = 0.0;
    TradeRecordList::const_iterator iter = m_trade_list.begin();
    for (; iter != m_trade_list.end(); ++iter) {
        //交易记录中的交易日期已经大于查询日期，则跳出循环
        if (iter->datetime > datetime) {
            break;
        }

        if (iter->stock == stock) {
            if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business) {
                number += iter->number;

            } else if (BUSINESS_SELL == iter->business) {
                number -= iter->number;

            } else {
                //其他情况忽略
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

bool TradeManager::checkin(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.3f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    //根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(cash, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.emplace_back(Null<Stock>(), datetime, BUSINESS_CHECKIN, in_cash, in_cash, 0.0, 0,
                              CostRecord(), 0.0, m_cash, 1.0, PART_INVALID);
    _saveAction(m_trade_list.back());
    return true;
}

bool TradeManager::checkout(const Datetime& datetime, price_t cash) {
    HKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ", datetime, cash);
    HKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                        "{} datetime must be >= lastDatetime({})!", datetime, lastDatetime());

    //根据权息调整当前持仓情况
    updateWithWeight(datetime);

    int precision = getParam<int>("precision");
    price_t out_cash = roundEx(cash, precision);
    HKU_ERROR_IF_RETURN(out_cash > m_cash, false,
                        "{} cash({:<.4f}) must be <= current cash({:<.4f})!", datetime, cash,
                        m_cash);

    m_cash = roundEx(m_cash - out_cash, precision);
    m_checkout_cash = roundEx(m_checkout_cash + out_cash, precision);
    m_trade_list.emplace_back(TradeRecord(Null<Stock>(), datetime, BUSINESS_CHECKOUT, out_cash,
                                          out_cash, 0.0, 0, CostRecord(), 0.0, m_cash, 1.0,
                                          PART_INVALID));
    _saveAction(m_trade_list.back());
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
    HKU_ERROR_IF_RETURN(number <= 0.0, result, "{} {} Invalid buy numer: {}!", datetime,
                        stock.market_code(), number);
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Buy number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                        "{} {} Buy number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

#if 0  //取消此处的检查，放松限制，另外也可以提高效率。另外，TM只负责交易管理，不许检查
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

    // 账户资金精度及保证金比例
    int precision = getParam<int>("precision");
    auto marginRatio = getMarginRatio(datetime, stock);
    CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

    // 买入资产价值
    price_t value = realPrice * number * stock.unit();

    // 计算买入当前资产需要的资金
    price_t total_need_money = roundEx(value * marginRatio + cost.total, precision);
    HKU_WARN_IF_RETURN(total_need_money > m_cash, result,
                       "{} {} Can't buy, need cash({:<.4f}) > current cash({:<.4f})!", datetime,
                       stock.market_code(), total_need_money, m_cash);

    // 更新现金
    m_cash = roundEx(m_cash - total_need_money, precision);

    // 加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice, goalPrice, number,
                         cost, stoploss, m_cash, marginRatio, from);
    m_trade_list.push_back(result);

    //更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord position;
        position.addTradeRecord(result);
        m_position[stock.id()] = std::move(position);
    } else {
        PositionRecord& position = pos_iter->second;
        position.addTradeRecord(result);
        if (position.number == 0.0) {
            m_position_history.push_back(std::move(position));
            m_position.erase(stock.id());
        }
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
    HKU_ERROR_IF_RETURN(number <= 0.0, result, "{} {} number is zero!", datetime,
                        stock.market_code());

    // 对于分红扩股造成不满足最小交易量整数倍的情况，只能通过number=MAX_DOUBLE的方式全仓卖出
    HKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                        "{} {} Sell number({}) must be >= minTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.minTradeNumber());
    HKU_ERROR_IF_RETURN(number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
                        "{} {} Sell number({}) must be <= maxTradeNumber({})!", datetime,
                        stock.market_code(), number, stock.maxTradeNumber());

    // 根据权息调整当前持仓情况
    updateWithWeight(datetime);

    // 获取当前持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), result,
                        "{} {} This stock was not bought never!", datetime, stock.market_code());

    PositionRecord& position = pos_iter->second;

    //调整欲卖出的数量，如果卖出数量等于MAX_DOUBLE，则表示卖出全部
    double real_number = number == MAX_DOUBLE ? position.number : number;

    //欲卖出的数量大于当前持仓的数量
    HKU_ERROR_IF_RETURN(position.number < real_number, result,
                        "{} {} Try to sell number({}) > number of position({})!", datetime,
                        stock.market_code(), real_number, position.number);

    int precision = getParam<int>("precision");
    CostRecord cost = getSellCost(datetime, stock, realPrice, real_number);
    auto margin = getMarginRatio(datetime, stock);

    // 更新当前持仓情况，并计算卖出后获利
    result = TradeRecord(stock, datetime, BUSINESS_SELL, planPrice, realPrice, goalPrice,
                         real_number, cost, stoploss, m_cash, 1.0, from);
    price_t profit = position.addTradeRecord(result);
    if (position.number == 0.0) {
        m_position_history.push_back(position);
        //删除当前持仓
        m_position.erase(stock.id());
    }

    //更新现金余额
    m_cash = roundEx(m_cash + profit - cost.total, precision);

    //更新交易记录
    result.cash = m_cash;
    m_trade_list.push_back(result);

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

FundsRecord TradeManager::_getFunds(KQuery::KType ktype) const {
    price_t value = 0.0;  //当前市值
    position_map_type::const_iterator iter = m_position.begin();
    if (!getParam<bool>("use_contract")) {
        for (; iter != m_position.end(); ++iter) {
            const PositionRecord& record = iter->second;
            price_t price = record.stock.getMarketValue(lastDatetime(), ktype);
            value += record.number * price * record.stock.unit();
        }
    } else {
        for (; iter != m_position.end(); ++iter) {
            value += iter->second.buyMoney;
        }
    }

    FundsRecord funds;
    funds.cash = m_cash;
    funds.market_value = roundEx(value, getParam<int>("precision"));
    funds.base_cash = m_checkin_cash - m_checkout_cash;
    funds.base_asset = m_checkin_stock - m_checkout_stock;
    return funds;
}

FundsRecord TradeManager::getFunds(const Datetime& indatetime, KQuery::KType ktype) {
    HKU_IF_RETURN(indatetime == Null<Datetime>(), _getFunds(ktype));
    HKU_IF_RETURN(indatetime < initDatetime(), FundsRecord());

    Datetime datetime(indatetime.year(), indatetime.month(), indatetime.day(), 23, 59);
    price_t market_value = 0.0;
    if (datetime >= lastDatetime()) {
        //根据权息数据调整持仓
        updateWithWeight(datetime);
        return _getFunds(ktype);
    }

    return _getFundsByDatetime(indatetime, ktype);

    // //当查询日期小于最后交易日期时，遍历交易记录，计算当日的市值和现金
    // price_t cash = m_init_cash;
    // struct Stock_Number {
    //     Stock_Number() : number(0) {}
    //     Stock_Number(const Stock& stock, size_t number) : stock(stock), number(number) {}

    //     Stock stock;
    //     size_t number;
    // };

    // price_t checkin_cash = 0.0;
    // price_t checkout_cash = 0.0;
    // price_t checkin_stock = 0.0;
    // price_t checkout_stock = 0.0;
    // map<uint64_t, Stock_Number> stock_map;
    // map<uint64_t, Stock_Number>::iterator stock_iter;

    // TradeRecordList::const_iterator iter = m_trade_list.begin();
    // for (; iter != m_trade_list.end(); ++iter) {
    //     if (iter->datetime > datetime) {
    //         //如果交易记录的日期大于指定的日期则跳出循环，处理完毕
    //         break;
    //     }

    //     cash = iter->cash;
    //     switch (iter->business) {
    //         case BUSINESS_INIT:
    //             checkin_cash += iter->realPrice;
    //             break;

    //         case BUSINESS_BUY:
    //         case BUSINESS_GIFT:
    //             stock_iter = stock_map.find(iter->stock.id());
    //             if (stock_iter != stock_map.end()) {
    //                 stock_iter->second.number += iter->number;
    //             } else {
    //                 stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
    //             }
    //             break;

    //         case BUSINESS_SELL:
    //             stock_iter = stock_map.find(iter->stock.id());
    //             if (stock_iter != stock_map.end()) {
    //                 stock_iter->second.number -= iter->number;
    //             } else {
    //                 HKU_WARN("{} {} Sell error in m_trade_list!", datetime,
    //                          iter->stock.market_code());
    //             }
    //             break;

    //         case BUSINESS_BONUS:
    //             break;

    //         case BUSINESS_CHECKIN:
    //             checkin_cash += iter->realPrice;
    //             break;

    //         case BUSINESS_CHECKOUT:
    //             checkout_cash += iter->realPrice;
    //             break;

    //         default:
    //             HKU_WARN("{} {} Unknown business in m_trade_list!", datetime,
    //                      iter->stock.market_code());
    //             break;
    //     }
    // }

    // stock_iter = stock_map.begin();
    // for (; stock_iter != stock_map.end(); ++stock_iter) {
    //     const size_t& number = stock_iter->second.number;
    //     if (number == 0) {
    //         continue;
    //     }

    //     price_t price = stock_iter->second.stock.getMarketValue(datetime, ktype);
    //     market_value = roundEx(market_value + price * number * stock_iter->second.stock.unit(),
    //                            getParam<int>("precision"));
    // }

    // FundsRecord funds;
    // funds.cash = cash;
    // funds.market_value = market_value;
    // funds.base_cash = checkin_cash - checkout_cash;
    // funds.base_asset = checkin_stock - checkout_stock;
    // return funds;
}

FundsRecord TradeManager::_getFundsByDatetime(const Datetime& datetime, KQuery::KType ktype) {
    TradeManager tm(m_init_datetime, m_init_cash, m_costfunc, m_mrfunc);
    for (const auto& tr : m_trade_list) {
        if (tr.datetime > datetime)
            break;

        switch (tr.business) {
            case BUSINESS_BUY:
                tm.buy(tr.datetime, tr.stock, tr.realPrice, tr.number, tr.stoploss, tr.goalPrice,
                       tr.planPrice, tr.from);
                break;

            case BUSINESS_SELL:
                tm.sell(tr.datetime, tr.stock, tr.realPrice, tr.number, tr.stoploss, tr.goalPrice,
                        tr.planPrice, tr.from);
                break;

            case BUSINESS_CHECKIN:
                tm.checkin(tr.datetime, tr.realPrice);
                break;

            case BUSINESS_CHECKOUT:
                tm.checkout(tr.datetime, tr.realPrice);
                break;

            case BUSINESS_INIT:
            case BUSINESS_GIFT:
            case BUSINESS_BONUS:
            case BUSINESS_INVALID:
            default:
                break;
        }
    }
    return tm._getFunds(ktype);
}

PriceList TradeManager::getFundsCurve(const DatetimeList& dates, KQuery::KType ktype) {
    size_t total = dates.size();
    PriceList result(total);
    HKU_IF_RETURN(total == 0, result);

    int precision = getParam<int>("precision");
    // for (size_t i = 0; i < total; ++i) {
    //     FundsRecord funds = getFunds(dates[i], ktype);
    //     result[i] = roundEx(funds.cash + funds.market_value, precision);
    // }

    // TradeManager tm(m_init_datetime, m_init_cash, m_costfunc, m_mrfunc);
    // tm.setParam<int>("precision", precision);

    // size_t tr_total = m_trade_list.size();
    // size_t pos = 0;
    // for (const auto& date : dates) {
    //     if (date < m_trade_list[pos].datetime) {
    //         for (const auto& position : tm.m_position) {
    //             position.second.stock.getMarketValue(date, ktype);
    //         }
    //     }
    // }

    StealThreadPool tg(std::thread::hardware_concurrency(), true);
    std::vector<std::future<FundsRecord>> tasks(total);
    Datetime date;
    for (size_t i = total - 1; i > 0; i--) {
        date = dates[i];
        tasks[i] = tg.submit([=]() { return getFunds(date, ktype); });
    }
    date = dates[0];
    tasks[0] = tg.submit([=]() { return getFunds(date, ktype); });

    FundsRecord funds;
    for (size_t i = 0; i < total; i++) {
        funds = tasks[i].get();
        result[i] = roundEx(funds.cash + funds.market_value, precision);
    }
    tg.join();

    return result;
}

PriceList TradeManager::getProfitCurve(const DatetimeList& dates, KQuery::KType ktype) {
    size_t total = dates.size();
    PriceList result(total);
    if (total == 0)
        return result;

    size_t i = 0;
    while (i < total && dates[i] < m_init_datetime) {
        result[i] = 0;
        i++;
    }
    int precision = getParam<int>("precision");
    for (; i < total; ++i) {
        FundsRecord funds = getFunds(dates[i], ktype);
        result[i] =
          roundEx(funds.cash + funds.market_value - funds.base_cash - funds.base_asset, precision);
    }

    return result;
}

void TradeManager::updateWithWeight(const Datetime& datetime) {
    HKU_IF_RETURN(datetime <= m_last_update_datetime, void());
    if (getParam<bool>("use_contract")) {
        _updateSettleByDay(datetime);
    } else {
        _updateWithWeight(datetime);
    }
    m_last_update_datetime = datetime;
}

/******************************************************************************
 *  每次执行交易操作时，先根据权息信息调整持有仓位及现金记录
 *  采用滞后更新的策略，即只在需要获取当前持仓情况及卖出时更新当前的持仓及资产情况
 *  输入参数： 本次操作的日期
 *  历史记录： 1) 2009/12/22 added
 *****************************************************************************/
void TradeManager::_updateWithWeight(const Datetime& datetime) {
    HKU_IF_RETURN(datetime <= m_last_update_datetime, void());

    //权息信息查询日期范围
    Datetime start_date(lastDatetime().date() + bd::days(1));
    Datetime end_date(datetime.date() + bd::days(1));

    int precision = getParam<int>("precision");
    price_t total_bonus = 0.0;
    price_t last_cash = m_cash;
    TradeRecordList new_trade_buffer;

    //更新持仓信息，并缓存新增的交易记录
    position_map_type::iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        PositionRecord& position = position_iter->second;
        Stock stock = position.stock;

        StockWeightList weights = stock.getWeight(start_date, end_date);
        StockWeightList::const_iterator weight_iter = weights.begin();
        for (; weight_iter != weights.end(); ++weight_iter) {
            //如果没有红利并且也（派股和转增股数量都为零），则跳过
            if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
                0.0 == weight_iter->increasement()) {
                continue;
            }

            //必须在加入配送股之前，因为配送股会引起持仓数量的变化
            if (weight_iter->bonus() != 0.0) {
                price_t bonus = roundEx(position.number * weight_iter->bonus() * 0.1, precision);
                position.sellMoney += bonus;
                last_cash += bonus;
                total_bonus += bonus;
                m_cash += bonus;
                new_trade_buffer.emplace_back(stock, weight_iter->datetime(), BUSINESS_BONUS, bonus,
                                              bonus, 0.0, 0, CostRecord(), 0.0, m_cash, 1.0,
                                              PART_INVALID);
            }

            size_t addcount =
              (position.number / 10.0) * (weight_iter->countAsGift() + weight_iter->increasement());
            if (addcount != 0.0) {
                position.number += addcount;
                new_trade_buffer.emplace_back(stock, weight_iter->datetime(), BUSINESS_GIFT, 0.0,
                                              0.0, 0.0, addcount, CostRecord(), 0.0, m_cash, 1.0,
                                              PART_INVALID);
            }
        } /* for weight */
    }     /* for position */

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
}

void TradeManager::_updateSettleByDay(const Datetime& datetime) {
    HKU_IF_RETURN(m_position.empty(), void());

    auto iter = m_position.begin();
    auto marginRatio = getMarginRatio(datetime, iter->second.stock);
    price_t totalProfit = 0.0;
    for (; iter != m_position.end(); ++iter) {
        totalProfit += iter->second.settleProfitOfPreDay(datetime, marginRatio);
    }

    // 将结算盈亏移入可用现金
    m_cash += roundEx(m_cash + totalProfit, getParam<int>("precision"));
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
                << m_costfunc->getParameter().getNameValueList() << "), "
                << "name='" << m_name << "'"
                << ")";
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
            buf << my_tm << "buy(Datetime('" << record.datetime.str() << "'), "
                << "sm['" << record.stock.market_code() << "'], " << record.realPrice << sep
                << record.number << sep << record.stoploss << sep << record.goalPrice << sep
                << record.planPrice << sep << record.from << ")";
            break;

        case BUSINESS_SELL:
            buf << my_tm << "sell(Datetime('" << record.datetime.str() << "'),"
                << "sm['" << record.stock.market_code() << "'], " << record.realPrice << sep
                << record.number << sep << record.stoploss << sep << record.goalPrice << sep
                << record.planPrice << sep << record.from << ")";
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

    //导出交易记录
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

    //导出已平仓记录
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
             << record.totalNumber << record.buyMoney << sep << record.totalCost << sep
             << record.sellMoney << sep << record.sellMoney - record.totalCost - record.buyMoney
             << sep << record.totalRisk << std::endl;
    }
    file.close();

    //导出未平仓记录
    file.open(filename3.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename3);
    file << "#建仓日期,平仓日期,证券代码,证券名称,当前持仓数量,"
            "累计花费资金,累计交易成本,已转化资金,累积风险,"
            "累计浮动盈亏,当前盈亏成本价"
         << std::endl;
    position_map_type::const_iterator position_iter = m_position.begin();
    for (; position_iter != m_position.end(); ++position_iter) {
        const PositionRecord& record = position_iter->second;
        file << record.takeDatetime << sep << record.cleanDatetime << sep
             << record.stock.market_code() << sep << record.stock.name() << sep << record.number
             << sep << record.buyMoney << sep << record.totalCost << sep << record.sellMoney << sep
             << record.totalRisk << sep;
        size_t pos = record.stock.getCount(KQuery::DAY);
        if (pos != 0) {
            KRecord krecord = record.stock.getKRecord(pos - 1, KQuery::DAY);
            price_t bonus = record.buyMoney - record.sellMoney - record.totalCost;
            file << record.number * krecord.closePrice - bonus << sep << bonus / record.number
                 << std::endl;
        }
    }
    file.close();

    //到处执行命令
    //导出已平仓记录
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

        case BUSINESS_INVALID:
        default:
            HKU_ERROR("tr.business is invalid({})!", tr.business);
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
                       "Don't have enough money!");

    m_cash = roundEx(m_cash - money - tr.cost.total, precision);
    new_tr.cash = m_cash;
    m_trade_list.push_back(new_tr);

    //更新当前持仓记录
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    if (pos_iter == m_position.end()) {
        PositionRecord position;
        position.addTradeRecord(tr);
        m_position[tr.stock.id()] = position;
    } else {
        PositionRecord& position = pos_iter->second;
        position.number += tr.number;
        position.totalNumber += tr.number;
        position.stoploss = tr.stoploss;
        position.goalPrice = tr.goalPrice;
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

    //未持仓
    position_map_type::iterator pos_iter = m_position.find(tr.stock.id());
    HKU_ERROR_IF_RETURN(pos_iter == m_position.end(), false, "No position!");

    PositionRecord& position = pos_iter->second;

    //欲卖出的数量大于当前持仓的数量
    HKU_ERROR_IF_RETURN(position.number < tr.number, false, "Try sell number greater position!");

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
    // position.buyMoney = position.buyMoney;
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

bool TradeManager::_add_checkin_tr(const TradeRecord& tr) {
    HKU_ERROR_IF_RETURN(tr.realPrice <= 0.0, false, "tr.realPrice <= 0.0!");
    int precision = getParam<int>("precision");
    price_t in_cash = roundEx(tr.realPrice, precision);
    m_cash = roundEx(m_cash + in_cash, precision);
    m_checkin_cash = roundEx(m_checkin_cash + in_cash, precision);
    m_trade_list.emplace_back(Null<Stock>(), tr.datetime, BUSINESS_CHECKIN, in_cash, in_cash, 0.0,
                              0, CostRecord(), 0.0, m_cash, 1.0, PART_INVALID);
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
    m_trade_list.emplace_back(Null<Stock>(), tr.datetime, BUSINESS_CHECKOUT, out_cash, out_cash,
                              0.0, 0, CostRecord(), 0.0, m_cash, 1.0, PART_INVALID);
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

} /* namespace hku */
