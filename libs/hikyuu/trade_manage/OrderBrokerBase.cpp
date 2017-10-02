/*
 * OrderBrokerBase.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include "OrderBrokerBase.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os, const OrderBrokerBase& broker) {
    os << "OrderBroker(" << broker.name() << ")" ;
    return os;
}

HKU_API std::ostream& operator <<(std::ostream &os, const OrderBrokerPtr& broker) {
    os << "OrderBroker(" << broker->name() << ")" ;
    return os;
}

OrderBrokerBase::OrderBrokerBase(): m_name("NO_NAME") {

}

OrderBrokerBase::OrderBrokerBase(const string& name): m_name(name) {

}

OrderBrokerBase::~OrderBrokerBase() {

}

Datetime OrderBrokerBase::buy(const string& code, price_t price, int num) {
    try {
        _buy(code, price, num);
    } catch (...) {
        HKU_ERROR("Unknow error in BUY operation!!! [OrderBrokerBase::buy]");
    }
    return Datetime::now();
}

Datetime OrderBrokerBase::sell(const string& code, price_t price, int num) {
    try {
        _sell(code, price, num);
    } catch (...) {
        HKU_ERROR("Unknow error in SELL operation!!! [OrderBrokerBase::sell]");
    }
    return Datetime::now();
}

} /* namespace hku */
