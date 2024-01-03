/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include "../GlobalInitializer.h"
#include <boost/algorithm/string.hpp>
#include "block_info/qianlong/QLBlockInfoDriver.h"
#include "kdata/cvs/KDataTempCsvDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

#if HKU_ENABLE_SQLITE_KDATA || HKU_ENABLE_HDF5_KDATA
#include "base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "block_info/sqlite/SQLiteBlockInfoDriver.h"
#endif

#if HKU_ENABLE_HDF5_KDATA
#include "kdata/hdf5/H5KDataDriver.h"
#endif

#if HKU_ENABLE_MYSQL_KDATA
#include "base_info/mysql/MySQLBaseInfoDriver.h"
#include "block_info/mysql/MySQLBlockInfoDriver.h"
#include "kdata/mysql/MySQLKDataDriver.h"
#endif

#if HKU_ENABLE_TDX_KDATA
#include "kdata/tdx/TdxKDataDriver.h"
#endif

#if HKU_ENABLE_SQLITE_KDATA
#include "kdata/sqlite/SQLiteKDataDriver.h"
#endif

namespace hku {

map<string, BaseInfoDriverPtr>* DataDriverFactory::m_baseInfoDrivers{nullptr};
map<string, BlockInfoDriverPtr>* DataDriverFactory::m_blockDrivers{nullptr};
map<string, KDataDriverPtr>* DataDriverFactory::m_kdataPrototypeDrivers{nullptr};

map<string, KDataDriverConnectPoolPtr>* DataDriverFactory::m_kdataDriverPools{nullptr};

void DataDriverFactory::init() {
    m_baseInfoDrivers = new map<string, BaseInfoDriverPtr>();
    m_blockDrivers = new map<string, BlockInfoDriverPtr>();
    DataDriverFactory::regBlockDriver(make_shared<QLBlockInfoDriver>());

#if HKU_ENABLE_SQLITE_KDATA || HKU_ENABLE_HDF5_KDATA
    DataDriverFactory::regBaseInfoDriver(make_shared<SQLiteBaseInfoDriver>());
    DataDriverFactory::regBlockDriver(make_shared<SQLiteBlockInfoDriver>());
#endif

#if HKU_ENABLE_MYSQL_KDATA
    DataDriverFactory::regBaseInfoDriver(make_shared<MySQLBaseInfoDriver>());
    DataDriverFactory::regBlockDriver(make_shared<MySQLBlockInfoDriver>());
#endif

    m_kdataPrototypeDrivers = new map<string, KDataDriverPtr>();
    m_kdataDriverPools = new map<string, KDataDriverConnectPoolPtr>();

    DataDriverFactory::regKDataDriver(make_shared<KDataTempCsvDriver>());

#if HKU_ENABLE_TDX_KDATA
    DataDriverFactory::regKDataDriver(make_shared<TdxKDataDriver>());
#endif

#if HKU_ENABLE_HDF5_KDATA
    DataDriverFactory::regKDataDriver(make_shared<H5KDataDriver>());
#endif

#if HKU_ENABLE_MYSQL_KDATA
    DataDriverFactory::regKDataDriver(make_shared<MySQLKDataDriver>());
#endif

#if HKU_ENABLE_SQLITE_KDATA
    DataDriverFactory::regKDataDriver(make_shared<SQLiteKDataDriver>());
#endif
}

void DataDriverFactory::release() {
    m_baseInfoDrivers->clear();
    delete m_baseInfoDrivers;
    m_baseInfoDrivers = nullptr;

    m_blockDrivers->clear();
    delete m_blockDrivers;
    m_blockDrivers = nullptr;

    m_kdataPrototypeDrivers->clear();
    delete m_kdataPrototypeDrivers;
    m_kdataPrototypeDrivers = nullptr;

    m_kdataDriverPools->clear();
    delete m_kdataDriverPools;
    m_kdataDriverPools = nullptr;
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
    (*m_kdataPrototypeDrivers)[new_type] = driver;
    // 不在此创建连接池
    //(*m_kdataDriverPools)[new_type] = make_shared<KDataDriverPool>();
}

void DataDriverFactory::removeKDataDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_kdataPrototypeDrivers->erase(new_name);
    auto iter = m_kdataDriverPools->find(new_name);
    if (iter != m_kdataDriverPools->end()) {
        m_kdataDriverPools->erase(iter);
    }
}

KDataDriverConnectPoolPtr DataDriverFactory::getKDataDriverPool(const Parameter& params) {
    KDataDriverConnectPoolPtr result;
    string name = params.get<string>("type");
    to_upper(name);
    auto iter = m_kdataDriverPools->find(name);
    if (iter != m_kdataDriverPools->end()) {
        result = iter->second;
    } else {
        auto prototype_iter = m_kdataPrototypeDrivers->find(name);
        HKU_CHECK(prototype_iter != m_kdataPrototypeDrivers->end(), "Unregistered driver：{}",
                  name);
        HKU_CHECK(prototype_iter->second->init(params), "Failed init driver: {}", name);
        (*m_kdataDriverPools)[name] = make_shared<KDataDriverConnectPool>(prototype_iter->second);
        result = (*m_kdataDriverPools)[name];
    }
    return result;
}

} /* namespace hku */
