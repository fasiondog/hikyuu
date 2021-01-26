/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "../StockManager.h"
#include "../base/GlobalTaskGroup.h"
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

bool BaseInfoDriver::loadBaseInfo() {
    HKU_IF_RETURN(!checkType(), false);
    auto& sm = StockManager::instance();

    HKU_INFO("Loading market information...");
    auto market_list = getAllMarketInfo();
    for (auto& market : market_list) {
        sm.loadMarketInfo(market);
    }

    HKU_INFO("Loading stock type information...");
    auto stktype_list = getAllStockTypeInfo();
    for (auto& stktype : stktype_list) {
        sm.loadStockTypeInfo(stktype);
    }

    HKU_INFO("Loading stock information...");
    HKU_FATAL_IF_RETURN(!_loadStock(), false, "Can't load Stock");

    HKU_INFO("Loading stock weight...");
    auto* tg = getGlobalTaskGroup();
    std::vector<std::future<void>> task_list;
    for (auto stock : sm) {
        task_list.push_back(tg->submit([=]() mutable {
            StockWeightList weightList = this->getStockWeightList(
              stock.market(), stock.code(), Datetime::min(), Null<Datetime>());
            stock.setWeightList(weightList);
        }));
    }
    // 权息信息如果不等待加载完毕，在数据加载期间进行计算可能导致复权错误，所以这里需要等待
    for (auto& task : task_list) {
        task.get();
    }

    return true;
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
