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
                               const Datetime& cleanDatetime, double number, price_t stoploss,
                               price_t goalPrice, double totalNumber, price_t totalMoney,
                               price_t totalCost, price_t totalRisk, price_t sellMoney)
: stock(stock),
  takeDatetime(takeDatetime),
  cleanDatetime(cleanDatetime),
  number(number),
  stoploss(stoploss),
  goalPrice(goalPrice),
  totalNumber(totalNumber),
  buyMoney(totalMoney),
  totalCost(totalCost),
  totalRisk(totalRisk),
  sellMoney(sellMoney) {}

void PositionRecord::addTradeRecord(const TradeRecord& tr) {
    HKU_ASSERT(tr.number != 0.0 && !tr.stock.isNull());
    if (m_trList.empty()) {
        stock = tr.stock;
        takeDatetime = tr.datetime;
    }
    m_isShort = m_trList.empty() && BUSINESS_SELL == tr.business ? true : false;
    takeDatetime = tr.datetime;
    number += tr.number;
    stoploss = tr.stoploss;
    goalPrice = tr.goalPrice;
    totalNumber += tr.number;
    buyMoney = roundEx(tr.realPrice * tr.number * stock.unit() * tr.margin_ratio + buyMoney,
                       stock.precision());
    totalCost = roundEx(tr.cost.total + totalCost, stock.precision());
    totalRisk =
      roundEx(totalRisk + (tr.realPrice - tr.stoploss) * number * stock.unit(), stock.precision());
    sellMoney = roundEx(sellMoney + tr.realPrice * tr.number * stock.unit() * tr.margin_ratio,
                        stock.precision());
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

    price_t costPrice = 0.0;
    if (number != 0.0) {
        costPrice = roundEx((buyMoney - sellMoney) / number, precision);
    }

    string strip(", ");
    std::stringstream os;
    os << std::fixed;
    os.precision(precision);
    os << "Position(" << market << strip << code << strip << name << strip << takeDatetime << strip
       << cleanDatetime << strip << number << strip << costPrice << strip << stoploss << strip
       << goalPrice << strip << totalNumber << strip << buyMoney << strip << totalCost << strip
       << totalRisk << strip << sellMoney << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2) {
    return d1.stock == d2.stock && d1.takeDatetime == d2.takeDatetime &&
           d1.cleanDatetime == d2.cleanDatetime && fabs(d1.number - d2.number) < 0.00001 &&
           fabs(d1.stoploss - d2.stoploss) < 0.0001 && fabs(d1.goalPrice - d2.goalPrice) < 0.0001 &&
           fabs(d1.totalNumber - d2.totalNumber) < 0.00001 &&
           fabs(d1.buyMoney - d2.buyMoney) < 0.0001 && fabs(d1.totalCost - d2.totalCost) < 0.0001 &&
           fabs(d1.sellMoney - d2.sellMoney) < 0.0001;
}

} /* namespace hku */
