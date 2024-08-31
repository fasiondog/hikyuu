/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "../StockManager.h"
#include "BaseInfoDriver.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const BaseInfoDriver& driver) {
    os << "BaseInfoDriver(" << driver.name() << ", " << driver.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const BaseInfoDriverPtr& driver) {
    if (driver) {
        os << *driver;
    } else {
        os << "BaseInfoDriver(NULL)";
    }

    return os;
}

BaseInfoDriver::BaseInfoDriver(const string& name) : m_name(name) {
    to_upper(m_name);
}

bool BaseInfoDriver::checkType() {
    bool result = false;
    try {
        string type = getParam<string>("type");
        to_upper(type);
        if (type == m_name) {
            result = true;
        } else {
            result = false;
            HKU_WARN("Type of driver mismatch! ({} != {})", type, m_name);
        }

    } catch (...) {
        result = false;
        HKU_ERROR("Can't get type of driver!");
    }

    return result;
}

bool BaseInfoDriver::init(const Parameter& params) {
    HKU_IF_RETURN(m_params == params, true);
    m_params = params;
    HKU_IF_RETURN(!checkType(), false);
    HKU_INFO("Using {} BaseInfoDriver", name());
    return _init();
}

Parameter BaseInfoDriver::getFinanceInfo(const string& market, const string& code) {
    HKU_INFO("The getFinanceInfo method has not been implemented! (BaseInfoDriver: {})", m_name);
    return Parameter();
}

StockWeightList BaseInfoDriver::getStockWeightList(const string& market, const string& code,
                                                   Datetime start, Datetime end) {
    HKU_INFO("The getStockWeightList method has not been implemented! (BaseInfoDriver: {})",
             m_name);
    return StockWeightList();
}

} /* namespace hku */
