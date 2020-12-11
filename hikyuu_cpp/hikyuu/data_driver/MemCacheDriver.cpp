/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-11
 *     Author: fasiondog
 */

#include "MemCacheDriver.h"

namespace hku {

MemCacheDriver::MemCacheDriver(const string& name) : m_name(name) {
    to_upper(m_name);
}

MemCacheDriver::~MemCacheDriver() {}

bool MemCacheDriver::checkType() {
    bool result = false;
    try {
        string type = getParam<string>("type");
        to_upper(type);
        HKU_CHECK(type == m_name, "Type of driver mismatch! ({} != {})", type, m_name);
        result = true;

    } catch (std::exception& e) {
        HKU_ERROR(e.what());

    } catch (...) {
        HKU_ERROR("Can't get type of driver!");
    }

    return result;
}

bool MemCacheDriver::init(const Parameter& params) {
    HKU_IF_RETURN(m_params == params, true);
    m_params = params;
    HKU_IF_RETURN(!checkType(), false);
    return _init();
}

}  // namespace hku