/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include "../GlobalInitializer.h"
#include <boost/algorithm/string.hpp>
#include "base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "base_info/mysql/MySQLBaseInfoDriver.h"
#include "block_info/qianlong/QLBlockInfoDriver.h"
#include "kdata/hdf5/H5KDataDriver.h"
#include "kdata/mysql/MySQLKDataDriver.h"
#include "kdata/tdx/TdxKDataDriver.h"
#include "kdata/hdf5/H5KDataDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

namespace hku {

map<string, BaseInfoDriverPtr>* DataDriverFactory::m_baseInfoDrivers;
map<string, BlockInfoDriverPtr>* DataDriverFactory::m_blockDrivers;
map<string, KDataDriverPtr>* DataDriverFactory::m_kdataDrivers;
map<Parameter, KDataDriverPtr>* DataDriverFactory::m_param_kdataDrivers;

void DataDriverFactory::init() {
    m_baseInfoDrivers = new map<string, BaseInfoDriverPtr>();
    DataDriverFactory::regBaseInfoDriver(make_shared<SQLiteBaseInfoDriver>());
    DataDriverFactory::regBaseInfoDriver(make_shared<MySQLBaseInfoDriver>());

    m_blockDrivers = new map<string, BlockInfoDriverPtr>();
    DataDriverFactory::regBlockDriver(make_shared<QLBlockInfoDriver>());

    m_kdataDrivers = new map<string, KDataDriverPtr>();
    m_param_kdataDrivers = new map<Parameter, KDataDriverPtr>();
    DataDriverFactory::regKDataDriver(make_shared<TdxKDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<H5KDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<MySQLKDataDriver>());
}

void DataDriverFactory::release() {
    m_baseInfoDrivers->clear();
    delete m_baseInfoDrivers;

    m_blockDrivers->clear();
    delete m_blockDrivers;

    m_kdataDrivers->clear();
    m_param_kdataDrivers->clear();
    delete m_kdataDrivers;
    delete m_param_kdataDrivers;
}

void DataDriverFactory::regBaseInfoDriver(const BaseInfoDriverPtr& driver) {
    HKU_CHECK(driver, "driver is nullptr!");
    string new_type(driver->name());
    to_upper(new_type);
    (*m_baseInfoDrivers)[new_type] = driver;
}

void DataDriverFactory::removeBaseInfoDriver(const string& name) {
    string new_type(name);
    to_upper(new_type);
    m_baseInfoDrivers->erase(new_type);
}

BaseInfoDriverPtr DataDriverFactory ::getBaseInfoDriver(const Parameter& params) {
    map<string, BaseInfoDriverPtr>::const_iterator iter;
    string type = params.get<string>("type");
    to_upper(type);
    iter = m_baseInfoDrivers->find(type);
    BaseInfoDriverPtr result;
    if (iter != m_baseInfoDrivers->end()) {
        result = iter->second;
        result->init(params);
    }
    return result;
}

void DataDriverFactory::regBlockDriver(const BlockInfoDriverPtr& driver) {
    HKU_CHECK(driver, "driver is nullptr!");
    string name(driver->name());
    to_upper(name);
    (*m_blockDrivers)[name] = driver;
}

void DataDriverFactory::removeBlockDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_blockDrivers->erase(new_name);
}

BlockInfoDriverPtr DataDriverFactory::getBlockDriver(const Parameter& params) {
    BlockInfoDriverPtr result;
    map<string, BlockInfoDriverPtr>::const_iterator iter;
    string name = params.get<string>("type");
    to_upper(name);
    iter = m_blockDrivers->find(name);
    if (iter != m_blockDrivers->end()) {
        result = iter->second;
        result->init(params);
    }

    return result;
}

void DataDriverFactory::regKDataDriver(const KDataDriverPtr& driver) {
    string new_type(driver->name());
    to_upper(new_type);
    (*m_kdataDrivers)[new_type] = driver;
}

void DataDriverFactory::removeKDataDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_kdataDrivers->erase(new_name);
    auto iter = m_param_kdataDrivers->begin();
    for (; iter != m_param_kdataDrivers->end(); ++iter) {
        string new_type(iter->first.get<string>("type"));
        to_upper(new_type);
        if (new_type == new_name) {
            break;
        }
    }

    if (iter != m_param_kdataDrivers->end()) {
        m_param_kdataDrivers->erase(iter);
    }
}

KDataDriverPtr DataDriverFactory::getKDataDriver(const Parameter& params) {
    KDataDriverPtr result;
    auto param_iter = m_param_kdataDrivers->find(params);
    if (param_iter != m_param_kdataDrivers->end()) {
        result = param_iter->second;
        return result;
    }

    string name;
    try {
        name = params.get<string>("type");
    } catch (...) {
        return result;
    }

    to_upper(name);
    auto iter = m_kdataDrivers->find(name);
    if (iter != m_kdataDrivers->end()) {
        result = iter->second;
        result->init(params);
        (*m_param_kdataDrivers)[params] = result;
    }

    return result;
}

} /* namespace hku */
