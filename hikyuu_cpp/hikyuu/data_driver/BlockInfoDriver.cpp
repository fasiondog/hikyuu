/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include "BlockInfoDriver.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const BlockInfoDriver& driver) {
    os << "BlockInfoDriver(" << driver.name() << ", " << driver.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const BlockInfoDriverPtr& driver) {
    if (driver) {
        os << *driver;
    } else {
        os << "BlockInfoDriver(NULL)";
    }

    return os;
}

BlockInfoDriver::BlockInfoDriver(const string& name) : m_name(name) {
    to_upper(m_name);
}

bool BlockInfoDriver::checkType() {
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

bool BlockInfoDriver::init(const Parameter& params) {
    HKU_IF_RETURN(m_params == params, true);
    m_params = params;
    HKU_IF_RETURN(!checkType(), false);
    return _init();
}

BlockList BlockInfoDriver::getStockBelongs(const Stock& stk, const string& category) {
    BlockList ret;
    auto category_blks = category.empty() ? getBlockList() : getBlockList(category);
    for (auto&& blk : category_blks) {
        if (blk.have(stk)) {
            ret.emplace_back(std::move(blk));
        }
    }
    return ret;
}

} /* namespace hku */
