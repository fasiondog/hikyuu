/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "block_info/qianlong/QLBlockInfoDriver.h"
#include "kdata/hdf5/H5KDataDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

namespace hku {

map<string, BaseInfoDriverPtr> default_baseinfo_driver() {
    map<string, BaseInfoDriverPtr> result;
    result["SQLITE3"] = make_shared<SQLiteBaseInfoDriver>();
    return result;
}

map<string, BlockInfoDriverPtr> default_block_driver() {
    map<string, BlockInfoDriverPtr> result;
    result["QIANLONG"] = make_shared<QLBlockInfoDriver>();
    return result;
}

map<string, KDataDriverPtr> default_kdata_driver() {
    map<string, KDataDriverPtr> result;
    result["HDF5"] = make_shared<H5KDataDriver>();
    return result;
}

map<string, BaseInfoDriverPtr> DataDriverFactory::m_baseInfoDrivers(default_baseinfo_driver());
map<string, BlockInfoDriverPtr> DataDriverFactory::m_blockDrivers(default_block_driver());
map<string, KDataDriverPtr> DataDriverFactory::m_kdataDrivers(default_kdata_driver());
map<Parameter, KDataDriverPtr> DataDriverFactory::m_param_kdataDrivers;


void DataDriverFactory
::regBaseInfoDriver(const BaseInfoDriverPtr& driver) {
    string new_type(driver->name());
    boost::to_upper(new_type);
    m_baseInfoDrivers[new_type] = driver;
}

void DataDriverFactory::removeBaseInfoDriver(const string& name) {
    string new_type(name);
    boost::to_upper(new_type);
    m_baseInfoDrivers.erase(new_type);
}

BaseInfoDriverPtr DataDriverFactory
::getBaseInfoDriver(const Parameter& params) {
    map<string, BaseInfoDriverPtr>::const_iterator iter;
    string type = params.get<string>("type");
    boost::to_upper(type);
    iter = m_baseInfoDrivers.find(type);
    BaseInfoDriverPtr result;
    if (iter != m_baseInfoDrivers.end()) {
        result = iter->second;
        result->init(params);
    }

    return result;
}

void DataDriverFactory::regBlockDriver(const BlockInfoDriverPtr& driver) {
    string name(driver->name());
    boost::to_upper(name);
    m_blockDrivers[name] = driver;
}

void DataDriverFactory::removeBlockDriver(const string& name) {
    string new_name(name);
    boost::to_upper(new_name);
    m_blockDrivers.erase(new_name);
}

BlockInfoDriverPtr DataDriverFactory::getBlockDriver(const Parameter& params) {
    BlockInfoDriverPtr result;
    map<string, BlockInfoDriverPtr>::const_iterator iter;
    string name = params.get<string>("type");
    boost::to_upper(name);
    iter = m_blockDrivers.find(name);
    if (iter != m_blockDrivers.end()) {
        result = iter->second;
        result->init(params);
    }

    return result;
}


void DataDriverFactory::regKDataDriver(const KDataDriverPtr& driver) {
    string new_type(driver->name());
    boost::to_upper(new_type);
    m_kdataDrivers[new_type] = driver;
}

void DataDriverFactory::removeKDataDriver(const string& name) {
    string new_name(name);
    boost::to_upper(new_name);
    m_kdataDrivers.erase(new_name);
    auto iter = m_param_kdataDrivers.begin();
    for (; iter != m_param_kdataDrivers.end(); ++iter) {
        string new_type(iter->first.get<string>("type"));
        boost::to_upper(new_type);
        if (new_type == new_name) {
            break;
        }
    }

    if (iter != m_param_kdataDrivers.end()) {
        m_param_kdataDrivers.erase(iter);
    }
}

KDataDriverPtr DataDriverFactory::getKDataDriver(const Parameter& params) {
    KDataDriverPtr result;
    auto param_iter = m_param_kdataDrivers.find(params);
    if (param_iter != m_param_kdataDrivers.end()) {
        result = param_iter->second;
        return result;
    }

    string name;
    try {
        name = params.get<string>("type");
    } catch (...) {
        return result;
    }

    boost::to_upper(name);
    auto iter = m_kdataDrivers.find(name);
    if (iter != m_kdataDrivers.end()) {
        result = iter->second;
        result->init(params);
        m_param_kdataDrivers[params] = result;
    }

    return result;
}


} /* namespace hku */
