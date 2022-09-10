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

void PositionRecord::addTradeRecord(const TradeRecord& tr, const MarginRecord& margin) {
    if (stock.isNull()) {
        stock = tr.stock;
        takeDatetime = tr.datetime;
    }
    switch (tr.business) {
        case BUSINESS_BUY: {
            double new_number = number + tr.number;
            if (new_number == 0.0) {
                cleanDatetime = tr.datetime;
            } else {
                avgPrice = std::fabs((tr.realPrice * tr.number + avgPrice * number) / new_number);
                number = new_number;
                stoploss = tr.stoploss;
                goalPrice = tr.goalPrice;
                totalNumber += tr.number;
                buyMoney =
                  roundEx(tr.realPrice * tr.number * stock.unit() * tr.margin_ratio + buyMoney,
                          stock.precision());
                totalCost = roundEx(tr.cost.total + totalCost, stock.precision());
                totalRisk =
                  roundEx(totalRisk + (tr.realPrice - tr.stoploss) * number * stock.unit(),
                          stock.precision());
            }
        } break;

        case BUSINESS_SELL: {
            double new_number = number - tr.number;
            if (new_number == 0.0) {
                cleanDatetime = tr.datetime;
            } else {
                avgPrice = std::fabs((tr.realPrice * tr.number - avgPrice * number) / new_number);
            }
            number = new_number;
            stoploss = tr.stoploss;
            goalPrice = tr.goalPrice;
            totalCost = roundEx(tr.cost.total + totalCost, stock.precision());
            totalRisk = roundEx(totalRisk + (tr.realPrice - tr.stoploss) * number * stock.unit(),
                                stock.precision());
            sellMoney =
              roundEx(sellMoney + tr.realPrice * tr.number * stock.unit(), stock.precision());

        } break;

        default:
            HKU_ERROR("The business({}) should not appear here!", getBusinessName(tr.business));
            break;
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
