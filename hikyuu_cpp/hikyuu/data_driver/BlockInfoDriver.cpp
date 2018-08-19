/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "BlockInfoDriver.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const BlockInfoDriver& driver) {
    os << "BlockInfoDriver(" << driver.name() << ", " << driver.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const BlockInfoDriverPtr& driver) {
    if (driver) {
        os << *driver;
    } else {
        os << "BlockInfoDriver(NULL)";
    }

    return os;
}

BlockInfoDriver::BlockInfoDriver(const string& name)
: m_name(name) {
    boost::to_upper(m_name);
}

bool BlockInfoDriver::checkType() {
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
                    << "[BlockInfoDriver::checkType]");
        }

    } catch(...) {
        result = false;
        HKU_ERROR("Can't get type of driver! [BlockInfoDriver::checkType]");
    }

    return result;
}

bool BlockInfoDriver::init(const Parameter& params) {
    if (m_params == params)
        return true;

    m_params = params;
    if (!checkType()) {
        return false;
    }
    return _init();
}

} /* namespace hku */
