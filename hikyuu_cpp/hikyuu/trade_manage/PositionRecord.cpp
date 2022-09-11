/*
 * PositionRecord.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include "PositionRecord.h"
#include "../utilities/util.h"

namespace hku {

PositionRecord::PositionRecord(const Stock& stock, const Datetime& takeDatetime,
                               const Datetime& cleanDatetime, double number, price_t avgPrice,
                               price_t stoploss, price_t goalPrice, double totalNumber,
                               price_t totalMoney, price_t totalCost, price_t totalRisk,
                               price_t sellMoney)
: stock(stock),
  takeDatetime(takeDatetime),
  cleanDatetime(cleanDatetime),
  number(number),
  avgPrice(avgPrice),
  stoploss(stoploss),
  goalPrice(goalPrice),
  totalNumber(totalNumber),
  buyMoney(totalMoney),
  totalCost(totalCost),
  totalRisk(totalRisk),
  sellMoney(sellMoney) {}

PositionRecord::PositionRecord(PositionRecord&& rv)
: stock(rv.stock),
  takeDatetime(rv.takeDatetime),
  cleanDatetime(rv.cleanDatetime),
  number(rv.number),
  avgPrice(rv.avgPrice),
  stoploss(rv.stoploss),
  goalPrice(rv.goalPrice),
  totalNumber(rv.totalNumber),
  buyMoney(rv.buyMoney),
  totalCost(rv.totalCost),
  totalRisk(rv.totalRisk),
  sellMoney(rv.sellMoney),
  contracts(std::move(rv.contracts)) {
    rv.stock = Null<Stock>();
}

PositionRecord& PositionRecord::operator=(PositionRecord&& rv) {
    HKU_IF_RETURN(this == &rv, *this);
    stock = rv.stock;
    takeDatetime = rv.takeDatetime;
    cleanDatetime = rv.cleanDatetime;
    number = rv.number;
    avgPrice = rv.avgPrice;
    stoploss = rv.stoploss;
    goalPrice = rv.goalPrice;
    totalNumber = rv.totalNumber;
    buyMoney = rv.buyMoney;
    totalCost = rv.totalCost;
    totalRisk = rv.totalRisk;
    sellMoney = rv.sellMoney;
    contracts = std::move(rv.contracts);
    rv.stock = Null<Stock>();
    return *this;
}

void PositionRecord::addTradeRecord(const TradeRecord& tr, const MarginRecord& margin) {
    HKU_ASSERT(tr.business == BUSINESS_BUY || tr.business == BUSINESS_SELL);
    if (stock.isNull()) {
        stock = tr.stock;
        takeDatetime = tr.datetime;
    }

    double tr_num = (tr.business == BUSINESS_BUY) ? tr.number : -tr.number;
    double new_number = number + tr_num;
    if (new_number < 0.0) {
        HKU_ERROR("The position number not match!");
        return;

    } else if (new_number == 0.0) {
        cleanDatetime = tr.datetime;
    }

    avgPrice = std::fabs((tr.realPrice * tr_num + avgPrice * number) / new_number);
    number = new_number;
    stoploss = tr.stoploss;
    goalPrice = tr.goalPrice;
    totalCost = roundEx(tr.cost.total + totalCost, stock.precision());
    totalRisk =
      roundEx(totalRisk + (tr.realPrice - tr.stoploss) * number * stock.unit(), stock.precision());

    if (tr.business == BUSINESS_BUY) {
        totalNumber += tr.number;
        buyMoney = roundEx(tr.realPrice * tr.number * stock.unit() * tr.margin_ratio + buyMoney,
                           stock.precision());
    } else {
        sellMoney = roundEx(sellMoney + tr.realPrice * tr.number * stock.unit(), stock.precision());
    }
}

string PositionRecord::toString() const {
    int precision = 2;
    std::string market(""), code(""), name("");
    if (!stock.isNull()) {
        market = stock.market();
        code = stock.code();
        name = stock.name();
    } else {
        precision = stock.precision();
    }

    string strip(", ");
    std::stringstream os;
    os << std::fixed;
    os.precision(precision);
    os << "Position(" << market << strip << code << strip << name << strip << takeDatetime << strip
       << cleanDatetime << strip << number << strip << avgPrice << strip << stoploss << strip
       << goalPrice << strip << totalNumber << strip << buyMoney << strip << totalCost << strip
       << totalRisk << strip << sellMoney << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2) {
    return d1.stock == d2.stock && d1.takeDatetime == d2.takeDatetime &&
           d1.cleanDatetime == d2.cleanDatetime && fabs(d1.number - d2.number) < 0.00001 &&
           fabs(d1.avgPrice - d2.avgPrice) < 0.0001 && fabs(d1.stoploss - d2.stoploss) < 0.0001 &&
           fabs(d1.goalPrice - d2.goalPrice) < 0.0001 &&
           fabs(d1.totalNumber - d2.totalNumber) < 0.00001 &&
           fabs(d1.buyMoney - d2.buyMoney) < 0.0001 && fabs(d1.totalCost - d2.totalCost) < 0.0001 &&
           fabs(d1.sellMoney - d2.sellMoney) < 0.0001;
}

} /* namespace hku */
