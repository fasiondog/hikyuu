/*
 * TradeRecord.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "hikyuu/global/sysinfo.h"
#include "TradeRecord.h"

namespace hku {

string HKU_API getBusinessName(BUSINESS business) {
    switch (business) {
        case BUSINESS_INIT:
            return "INIT";
        case BUSINESS_BUY:
            return "BUY";
        case BUSINESS_SELL:
            return "SELL";
        case BUSINESS_GIFT:
            return "GIFT";
        case BUSINESS_BONUS:
            return "BONUS";
        case BUSINESS_CHECKIN:
            return "CHECKIN";
        case BUSINESS_CHECKOUT:
            return "CHECKOUT";
        case BUSINESS_CHECKIN_STOCK:
            return "CHECKIN_STOCK";
        case BUSINESS_CHECKOUT_STOCK:
            return "CHECKOUT_STOCK";
        case BUSINESS_BORROW_CASH:
            return "BORROW_CASH";
        case BUSINESS_RETURN_CASH:
            return "RETURN_CASH";
        case BUSINESS_BORROW_STOCK:
            return "BORROW_STOCK";
        case BUSINESS_RETURN_STOCK:
            return "RETURN_STOCK";
        case BUSINESS_SELL_SHORT:
            return "SELL_SHORT";
        case BUSINESS_BUY_SHORT:
            return "BUY_SHORT";
        default:
            return "UNKNOWN";
    }
}

BUSINESS HKU_API getBusinessEnum(const string& arg) {
    BUSINESS business;
    string business_name(arg);
    to_upper(business_name);
    if (business_name == "INIT") {
        business = BUSINESS_INIT;
    } else if (business_name == "BUY") {
        business = BUSINESS_BUY;
    } else if (business_name == "SELL") {
        business = BUSINESS_SELL;
    } else if (business_name == "GIFT") {
        business = BUSINESS_GIFT;
    } else if (business_name == "BONUS") {
        business = BUSINESS_BONUS;
    } else if (business_name == "CHECKIN") {
        business = BUSINESS_CHECKIN;
    } else if (business_name == "CHECKOUT") {
        business = BUSINESS_CHECKOUT;
    } else if (business_name == "CHECKIN_STOCK") {
        return BUSINESS_CHECKIN_STOCK;
    } else if (business_name == "CHECKOUT_STOCK") {
        return BUSINESS_CHECKOUT_STOCK;
    } else if (business_name == "BORROW_CASH") {
        return BUSINESS_BORROW_CASH;
    } else if (business_name == "RETURN_CASH") {
        return BUSINESS_RETURN_CASH;
    } else if (business_name == "BORROW_STOCK") {
        return BUSINESS_BORROW_STOCK;
    } else if (business_name == "RETURN_STOCK") {
        return BUSINESS_RETURN_STOCK;
    } else if (business_name == "SELL_SHORT") {
        return BUSINESS_SELL_SHORT;
    } else if (business_name == "BUY_SHORT") {
        return BUSINESS_BUY_SHORT;
    } else {
        business = BUSINESS_INVALID;
    }
    return business;
}

TradeRecord::TradeRecord()
: business(BUSINESS_INVALID),
  planPrice(0.0),
  realPrice(0.0),
  goalPrice(0.0),
  number(0.0),
  stoploss(0.0),
  cash(0.0),
  from(PART_INVALID) {}

TradeRecord::TradeRecord(const Stock& stock, const Datetime& datetime, BUSINESS business,
                         price_t planPrice, price_t realPrice, price_t goalPrice, double number,
                         const CostRecord& cost, price_t stoploss, price_t cash, SystemPart from)
: stock(stock),
  datetime(datetime),
  business(business),
  planPrice(planPrice),
  realPrice(realPrice),
  goalPrice(goalPrice),
  number(number),
  cost(cost),
  stoploss(stoploss),
  cash(cash),
  from(from) {}

HKU_API std::ostream& operator<<(std::ostream& os, const TradeRecord& record) {
    os << record.toString();
    return os;
}

string TradeRecord::toString() const {
    string market_code(""), name("");
    if (!stock.isNull()) {
        market_code = stock.market_code();
        name = stock.name();
    }

#if HKU_OS_WINDOWS
    return fmt::format("Trade({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})", datetime,
                       market_code, runningInPython() && pythonInJupyter() ? name : UTF8ToGB(name),
                       getBusinessName(business), planPrice, realPrice, goalPrice, number,
                       cost.commission, cost.stamptax, cost.transferfee, cost.others,
                       getSystemPartName(from));
#else
    return fmt::format("Trade({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})", datetime,
                       market_code, name, getBusinessName(business), planPrice, realPrice,
                       goalPrice, number, cost.commission, cost.stamptax, cost.transferfee,
                       cost.others, getSystemPartName(from));
#endif
}

bool TradeRecord::isNull() const {
    return business == BUSINESS_INVALID;
}

bool HKU_API operator==(const TradeRecord& d1, const TradeRecord& d2) {
    return d1.stock == d2.stock && d1.datetime == d2.datetime && d1.business == d2.business &&
           fabs(d1.planPrice - d2.planPrice) < 0.0001 &&
           fabs(d1.realPrice - d2.realPrice) < 0.0001 &&
           fabs(d1.goalPrice - d2.goalPrice) < 0.0001 && fabs(d1.number - d2.number) < 0.000001 &&
           d1.cost == d2.cost && fabs(d1.stoploss - d2.stoploss) < 0.0001 &&
           fabs(d1.cash - d2.cash) < 0.0001 && d1.from == d2.from;
}

} /* namespace hku */
