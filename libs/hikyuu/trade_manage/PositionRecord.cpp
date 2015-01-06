/*
 * PositionRecord.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include "PositionRecord.h"
#include "../utilities/util.h"

namespace hku {

PositionRecord::PositionRecord()
: number(0), stoploss(0.0), goalPrice(0.0), totalNumber(0), totalMoney(0.0),
  totalCost(0.0), totalRisk(0.0), sellMoney(0.0) {
}


PositionRecord::PositionRecord(
        const Stock& stock,
        const Datetime& takeDatetime,
        const Datetime& cleanDatetime,
        size_t  number,
        price_t stoploss,
        price_t goalPrice,
        size_t  totalNumber,
        price_t totalMoney,
        price_t totalCost,
        price_t totalRisk,
        price_t sellMoney)
: stock(stock), takeDatetime(takeDatetime), cleanDatetime(cleanDatetime),
  number(number), stoploss(stoploss), goalPrice(goalPrice),
  totalNumber(totalNumber), totalMoney(totalMoney),
  totalCost(totalCost), totalRisk(totalRisk), sellMoney(sellMoney) {

}


HKU_API std::ostream & operator<<(std::ostream& os, const PositionRecord& record) {
    Stock stock = record.stock;
    int precision = 2;
    std::string market(""), code(""), name("");
    if(stock.isNull()){
        market = stock.market();
        code = stock.code();
        name = stock.name();
    } else {
        precision = stock.precision();
    }

    price_t costPrice = 0.0;
    if (record.number != 0.0) {
        costPrice = roundEx((record.totalMoney - record.sellMoney)
                / record.number, precision);
    }

    string strip(", ");
    os << std::fixed;
    os.precision(precision);
    os << "Position(" << market
            << strip << code
            << strip << name
            << strip << record.takeDatetime
            << strip << record.cleanDatetime
            << strip << record.number
            << strip << costPrice
            << strip << record.stoploss
            << strip << record.goalPrice
            << strip << record.totalNumber
            << strip << record.totalMoney
            << strip << record.totalCost
            << strip << record.totalRisk
            << strip << record.sellMoney << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}


bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2) {
    if (d1.stock == d2.stock
            && d1.takeDatetime == d2.takeDatetime
            && d1.cleanDatetime == d2.cleanDatetime
            && d1.number == d2.number
            && fabs(d1.stoploss - d2.stoploss) < 0.0001
            && fabs(d1.goalPrice - d2.goalPrice) < 0.0001
            && d1.totalNumber == d2.totalNumber
            && fabs(d1.totalMoney - d2.totalMoney) < 0.0001
            && fabs(d1.totalCost - d2.totalCost) < 0.0001
            && fabs(d1.sellMoney - d2.sellMoney) < 0.0001) {
        return true;
    }
    return false;
}

} /* namespace hku */
