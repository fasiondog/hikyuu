/*
 * OrderBrokerBase.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include "OrderBrokerBase.h"

namespace hku {

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

string OrderBrokerBase::buy(Datetime datetime, const string& market, const string& code,
                            price_t price, double num) {
    string ret;
    try {
        ret = _buy(datetime, market, code, price, num);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
    return ret;
}

string OrderBrokerBase::sell(Datetime datetime, const string& market, const string& code,
                             price_t price, double num) {
    string ret;
    try {
        ret = _sell(datetime, market, code, price, num);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
    return ret;
}

} /* namespace hku */
