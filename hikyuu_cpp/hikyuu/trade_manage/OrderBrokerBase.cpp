/*
 * OrderBrokerBase.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "OrderBrokerBase.h"

namespace hku {

using json = nlohmann::json;

BrokerPositionRecord::BrokerPositionRecord(const Stock& stock_, price_t number_, price_t money_)
: stock(stock_), number(number_), money(money_) {}

BrokerPositionRecord::BrokerPositionRecord(BrokerPositionRecord&& rv)
: stock(std::move(rv.stock)), number(rv.number), money(rv.money) {
    rv.number = 0.0;
    rv.money = 0.0;
}

BrokerPositionRecord& BrokerPositionRecord::operator=(BrokerPositionRecord&& rv) {
    if (this != &rv) {
        stock = std::move(rv.stock);
        number = rv.number;
        money = rv.money;
        rv.number = 0.0;
        rv.money = 0.0;
    }
    return *this;
}

string BrokerPositionRecord::str() const {
    return fmt::format("BrokerPositionRecord({}, {:<.4f}, {:<.4f})", stock.market_code(), number,
                       money);
}

HKU_API std::ostream& operator<<(std::ostream& os, const BrokerPositionRecord& pos) {
    os << pos.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerBase& broker) {
    os << "OrderBroker(" << broker.name() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerPtr& broker) {
    os << "OrderBroker(" << broker->name() << ")";
    return os;
}

OrderBrokerBase::OrderBrokerBase() : m_name("NO_NAME") {}

OrderBrokerBase::OrderBrokerBase(const string& name) : m_name(name) {}

OrderBrokerBase::~OrderBrokerBase() {}

void OrderBrokerBase::buy(Datetime datetime, const string& market, const string& code,
                          price_t price, double num, price_t stoploss, price_t goalPrice,
                          SystemPart from) noexcept {
    try {
        _buy(datetime, market, code, price, num, stoploss, goalPrice, from);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
}

void OrderBrokerBase::sell(Datetime datetime, const string& market, const string& code,
                           price_t price, double num, price_t stoploss, price_t goalPrice,
                           SystemPart from) noexcept {
    try {
        _sell(datetime, market, code, price, num, stoploss, goalPrice, from);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
}

string OrderBrokerBase::getAssetInfo() noexcept {
    string ret;
    try {
        ret = _getAssetInfo();
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
    return ret;
}

} /* namespace hku */
