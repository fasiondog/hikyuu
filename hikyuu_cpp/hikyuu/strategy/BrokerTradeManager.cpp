/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-16
 *      Author: fasiondog
 */

#include "hikyuu/trade_manage/crt/TC_Zero.h"
#include "BrokerTradeManager.h"

namespace hku {

BrokerTradeManager::BrokerTradeManager(const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                       const string& name)
: TradeManagerBase(name, costfunc) {
    HKU_ASSERT(broker);
    m_broker_list.emplace_back(broker);

    m_init_cash = broker->cash();
    m_cash = m_init_cash;

    auto now = Datetime::now();
    auto brk_positions = broker->position();
    for (const auto& brk_pos : brk_positions) {
        PositionRecord pos;
        pos.takeDatetime = now;
        pos.stock = brk_pos.stock;
        pos.number = brk_pos.number;
        pos.totalNumber = brk_pos.number;
        pos.buyMoney = brk_pos.money;
        pos.totalRisk = brk_pos.money;
        m_position[pos.stock.id()] = pos;
    }

    m_init_datetime = Datetime::now();
    m_first_datetime = m_init_datetime;
    m_last_datetime = m_init_datetime;
    m_broker_last_datetime = m_init_datetime;
}

void BrokerTradeManager::_reset() {
    HKU_WARN("The subclass does not implement a reset method");
    m_first_datetime = m_init_datetime;
    m_last_datetime = m_init_datetime;
    m_cash = m_init_cash;
    m_position.clear();
}

shared_ptr<TradeManagerBase> BrokerTradeManager::_clone() {
    BrokerTradeManager* p = new BrokerTradeManager();
    p->m_init_datetime = m_init_datetime;
    p->m_first_datetime = m_first_datetime;
    p->m_last_datetime = m_last_datetime;
    p->m_init_cash = m_init_cash;
    p->m_cash = m_cash;
    p->m_position = m_position;
    return shared_ptr<TradeManagerBase>(p);
}

void BrokerTradeManager::getCashFromBroker() {
    m_cash = m_broker_list.front()->cash();
}

void BrokerTradeManager::getPositionFromBroker() {
    auto& broker = m_broker_list.front();

    auto now = Datetime::now();
    auto brk_positions = broker->position();

    position_map_type new_positions;
    for (const auto& brk_pos : brk_positions) {
        PositionRecord pos;
        pos.takeDatetime = now;
        pos.stock = brk_pos.stock;
        auto iter = m_position.find(pos.stock.id());
        if (iter == m_position.end()) {
            pos.number = brk_pos.number;
            pos.totalNumber = brk_pos.number;
            pos.buyMoney = brk_pos.money;
            pos.totalRisk = brk_pos.money;
            new_positions[pos.stock.id()] = pos;
        } else {
            auto& cur_pos = iter->second;
            if (cur_pos.number != 0.0) {
                pos.totalCost = cur_pos.totalCost / cur_pos.number * brk_pos.number;
                pos.totalRisk = cur_pos.totalRisk / cur_pos.number * brk_pos.number;
            } else {
                pos.totalRisk = brk_pos.money;
            }
            pos.number = brk_pos.number;
            pos.totalNumber = brk_pos.number;
            pos.buyMoney = pos.buyMoney;
            new_positions[pos.stock.id()] = pos;
        }
    }

    m_position.swap(new_positions);

    m_init_datetime = Datetime::now();
    m_first_datetime = m_init_datetime;
    m_last_datetime = m_init_datetime;
    m_broker_last_datetime = m_init_datetime;
}

PositionRecordList BrokerTradeManager::getPositionList() const {
    PositionRecordList result;
    position_map_type::const_iterator iter = m_position.begin();
    for (; iter != m_position.end(); ++iter) {
        result.push_back(iter->second);
    }
    return result;
}

bool BrokerTradeManager::checkin(const Datetime& datetime, price_t cash) {
    HKU_IF_RETURN(datetime < m_last_datetime, false);
    m_cash += cash;
    return true;
}

TradeRecord BrokerTradeManager::buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                    double number, price_t stoploss, price_t goalPrice,
                                    price_t planPrice, SystemPart from) {
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

    // 同步资金与账户信息
    getCashFromBroker();
    getPositionFromBroker();

    CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

    // 实际交易需要的现金＝交易数量＊实际交易价格＋交易总成本
    int precision = getParam<int>("precision");
    // price_t money = roundEx(realPrice * number * stock.unit() + cost.total, precision);
    price_t money = roundEx(realPrice * number * stock.unit(), precision);

    HKU_WARN_IF_RETURN(m_cash < roundEx(money + cost.total, precision), result,
                       "{} {} Can't buy, need cash({:<.4f}) > current cash({:<.4f})!", datetime,
                       stock.market_code(), roundEx(money + cost.total, precision), m_cash);

    // 更新现金
    m_cash = roundEx(m_cash - money - cost.total, precision);

    // 加入交易记录
    result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice, goalPrice, number,
                         cost, stoploss, m_cash, from);

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

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->buy(datetime, stock.market(), stock.code(), realPrice, number, stoploss, goalPrice,
                    from);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    return result;
}

TradeRecord BrokerTradeManager::sell(const Datetime& datetime, const Stock& stock,
                                     price_t realPrice, double number, price_t stoploss,
                                     price_t goalPrice, price_t planPrice, SystemPart from) {
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

    // 同步资金与账户信息
    getCashFromBroker();
    getPositionFromBroker();

    // 未持仓
    position_map_type::iterator pos_iter = m_position.find(stock.id());
    HKU_TRACE_IF_RETURN(pos_iter == m_position.end(), result,
                        "{} {} This stock was not bought never! ({}, {:<.4f}, {}, {})", datetime,
                        stock.market_code(), datetime, realPrice, number, getSystemPartName(from));

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

    // 更新当前持仓情况
    position.number -= real_number;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    // position.buyMoney = position.buyMoney;
    position.totalCost = roundEx(position.totalCost + cost.total, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);

    if (position.number == 0) {
        // 删除当前持仓
        m_position.erase(stock.id());
    }

    if (datetime > m_broker_last_datetime) {
        list<OrderBrokerPtr>::const_iterator broker_iter = m_broker_list.begin();
        for (; broker_iter != m_broker_list.end(); ++broker_iter) {
            (*broker_iter)
              ->sell(datetime, stock.market(), stock.code(), realPrice, real_number, stoploss,
                     goalPrice, from);
            if (datetime > m_broker_last_datetime) {
                m_broker_last_datetime = datetime;
            }
        }
    }

    return result;
}

FundsRecord BrokerTradeManager::getFunds(KQuery::KType inktype) const {
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

    funds.cash = m_cash;
    funds.market_value = value;
    funds.short_market_value = 0.0;
    funds.base_cash = m_init_cash;
    funds.base_asset = 0.0;
    funds.borrow_cash = 0.0;
    funds.borrow_asset = 0.0;
    return funds;
}

FundsRecord BrokerTradeManager::getFunds(const Datetime& datetime, KQuery::KType ktype) {
    return (datetime >= m_last_datetime) ? getFunds(ktype) : FundsRecord();
}

}  // namespace hku