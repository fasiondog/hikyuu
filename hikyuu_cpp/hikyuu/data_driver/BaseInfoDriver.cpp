/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "BaseInfoDriver.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const BaseInfoDriver& driver) {
    os << "BaseInfoDriver(" << driver.name() << ", " << driver.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const BaseInfoDriverPtr& driver) {
    if (driver) {
        os << *driver;
    } else {
        os << "BaseInfoDriver(NULL)";
    }

    return os;
}

BaseInfoDriver::BaseInfoDriver(const string& name)
: m_name(name) {
    boost::to_upper(m_name);
}

bool BaseInfoDriver::checkType() {
    bool result = false;
    try {
        string type = getParam<string>("type");
        boost::to_upper(type);
        if (type == m_name) {
            result = true;
        } else {
            result = false;
            HKU_WARN("Type of driver mismatch! ("
                    << type << " != " << m_name << ") "
                    << "[BaseInfoDriver::checkType]");
        }

    } catch(...) {
        result = false;
        HKU_ERROR("Can't get type of driver! [BaseInfoDriver::checkType]");
    }

    return result;
}

bool BaseInfoDriver::init(const Parameter& params) {
    if (m_params == params)
        return true;

    m_params = params;
    if (!checkType()) {
        return false;
    }
    return _init();
}

bool BaseInfoDriver::loadBaseInfo() {
    if (!checkType()) {
        return false;
    }

    HKU_TRACE("Loading market information...");
    if (!_loadMarketInfo()) {
        HKU_FATAL("Can't load Market Information.");
        return false;
    }

    HKU_TRACE("Loading stock type information...");
    if (!_loadStockTypeInfo()) {
        HKU_FATAL("Can't load StockType Information.");
        return false;
    }

    HKU_TRACE("Loading stock information...");
    if (!_loadStock()) {
        HKU_FATAL("Can't load Stock");
        return false;
    }

    return true;
}

} /* namespace hku */
