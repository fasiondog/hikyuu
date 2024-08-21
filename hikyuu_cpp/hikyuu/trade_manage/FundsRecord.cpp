/*
 * FundsRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include "FundsRecord.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const FundsRecord& funds) {
    os << std::fixed;
    os.precision(4);

    string strip(", ");
    os << "FundsRecord(" << funds.cash << strip << funds.market_value << strip
       << funds.short_market_value << strip << funds.base_cash << strip << funds.base_asset << strip
       << funds.borrow_cash << strip << funds.borrow_asset << ")";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

FundsRecord::FundsRecord(price_t cash, price_t market_value, price_t short_market_value,
                         price_t base_cash, price_t base_asset, price_t borrow_cash,
                         price_t borrow_asset)
: cash(cash),
  market_value(market_value),
  short_market_value(short_market_value),
  base_cash(base_cash),
  base_asset(base_asset),
  borrow_cash(borrow_cash),
  borrow_asset(borrow_asset) {}

FundsRecord FundsRecord::operator+(const FundsRecord& other) const {
    FundsRecord result;
    result.cash = cash + other.cash;
    result.market_value = market_value + other.market_value;
    result.short_market_value = short_market_value + other.short_market_value;
    result.base_cash = base_cash + other.base_cash;
    result.base_asset = base_asset + other.base_asset;
    result.borrow_cash = borrow_cash + other.borrow_cash;
    result.borrow_asset = borrow_asset + other.borrow_asset;
    return result;
}

FundsRecord& FundsRecord::operator+=(const FundsRecord& other) {
    cash += other.cash;
    market_value += other.market_value;
    short_market_value += other.short_market_value;
    base_cash += other.base_cash;
    base_asset += other.base_asset;
    borrow_cash += other.borrow_cash;
    borrow_asset += other.borrow_asset;
    return *this;
}

bool HKU_API operator==(const FundsRecord& d1, const FundsRecord& d2) {
    return (std::fabs(d1.cash - d2.cash) < 0.0001 &&
            std::fabs(d1.market_value - d2.market_value) < 0.0001 &&
            std::fabs(d1.short_market_value - d2.short_market_value) < 0.0001 &&
            std::fabs(d1.base_cash - d2.base_cash) < 0.0001 &&
            std::fabs(d1.base_asset - d2.base_asset) < 0.0001 &&
            std::fabs(d1.borrow_cash - d2.borrow_cash) < 0.0001 &&
            std::fabs(d1.borrow_asset - d2.borrow_asset) < 0.0001);
}

} /* namespace hku */
