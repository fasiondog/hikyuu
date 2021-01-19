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
#include "kdata/cvs/KDataTempCsvDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

namespace hku {

map<string, BaseInfoDriverPtr>* DataDriverFactory::m_baseInfoDrivers{nullptr};
map<string, BlockInfoDriverPtr>* DataDriverFactory::m_blockDrivers{nullptr};
map<string, KDataDriverPtr>* DataDriverFactory::m_kdataDrivers{nullptr};

map<string, DataDriverFactory::KDataDriverPool*>* DataDriverFactory::m_kdataDriverPools{nullptr};

void DataDriverFactory::init() {
    m_baseInfoDrivers = new map<string, BaseInfoDriverPtr>();
    DataDriverFactory::regBaseInfoDriver(make_shared<SQLiteBaseInfoDriver>());
    DataDriverFactory::regBaseInfoDriver(make_shared<MySQLBaseInfoDriver>());

    m_blockDrivers = new map<string, BlockInfoDriverPtr>();
    DataDriverFactory::regBlockDriver(make_shared<QLBlockInfoDriver>());

    m_kdataDriverPools = new map<string, KDataDriverPool*>();

    m_kdataDrivers = new map<string, KDataDriverPtr>();
    DataDriverFactory::regKDataDriver(make_shared<TdxKDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<H5KDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<MySQLKDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<KDataTempCsvDriver>());
}

void DataDriverFactory::release() {
    m_baseInfoDrivers->clear();
    delete m_baseInfoDrivers;

    m_blockDrivers->clear();
    delete m_blockDrivers;

    m_kdataDrivers->clear();
    delete m_kdataDrivers;

    for (auto iter = m_kdataDriverPools->begin(); iter != m_kdataDriverPools->end(); ++iter) {
        delete iter->second;
    }
    m_kdataDriverPools->clear();
    delete m_kdataDriverPools;
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
    HKU_CHECK(m_kdataDriverPools->find(new_type) == m_kdataDriverPools->end(),
              "Repeat regKDataDriver!");
    (*m_kdataDrivers)[new_type] = driver;
    (*m_kdataDriverPools)[new_type] = new KDataDriverPool(driver);
}

void DataDriverFactory::removeKDataDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_kdataDrivers->erase(new_name);
    auto iter = m_kdataDriverPools->find(new_name);
    if (iter != m_kdataDriverPools->end()) {
        delete iter->second;
        m_kdataDriverPools->erase(iter);
    }
}

KDataDriverPtr DataDriverFactory::getKDataDriver(const Parameter& params) {
    KDataDriverPtr result;
    string name = params.get<string>("type");
    to_upper(name);
    auto iter = m_kdataDrivers->find(name);
    if (iter != m_kdataDrivers->end()) {
        result = iter->second;
        result->init(params);
    }
    return result;
}

DataDriverFactory::KDataDriverPool* DataDriverFactory::getKDataDriverPool(const Parameter& params) {
    KDataDriverPool* result;
    string name = params.get<string>("type");
    to_upper(name);
    auto iter = m_kdataDriverPools->find(name);
    if (iter != m_kdataDriverPools->end()) {
        result = iter->second;
        auto prototype = result->getPrototype();
        prototype->init(params);
    }
    return result;
}

} /* namespace hku */
