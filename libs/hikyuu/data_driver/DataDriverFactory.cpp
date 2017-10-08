/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "base_info/mysql/MySQLBaseInfoDriver.h"
#include "base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "block_info/qianlong/QLBlockInfoDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

namespace hku {

//DataDriverFactory::DataDriverFactory() {
//    m_baseInfoDrivers["SQLITE3"] = make_shared<SQLiteBaseInfoDriver>();
//}

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

map<string, BaseInfoDriverPtr> DataDriverFactory::m_baseInfoDrivers(default_baseinfo_driver());
map<string, BlockInfoDriverPtr> DataDriverFactory::m_blockDrivers(default_block_driver());

KDataDriverPtr DataDriverFactory::
getKDataDriver(const shared_ptr<IniParser>& ini) {
    string func_name(" [DataDriverFactory::getKDataDriver]");
    KDataDriverPtr  result;
    if (!ini) {
        HKU_ERROR("Null configure ini!" << func_name);
        return result;
    }

    result = make_shared<KDataDriver>(ini);
    return result;
}

/*BlockInfoDriverPtr DataDriverFactory::
getBlockDriver(const shared_ptr<IniParser>& ini) {
    string func_name(" [DataDriverFactory::getBlockDriver]");
    BlockInfoDriverPtr result;
    if (!ini) {
        HKU_ERROR("Null configure ini!" << func_name);
        return result;
    }

    if (!ini->hasOption("block", "type")) {
        return result;
    }

    string type = ini->get("block", "type");
    if (type == "qianlong") {
        result = make_shared<QLBlockInfoDriver>(ini);
    }
    return result;
}*/

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
    } else {
        HKU_INFO("Can't get the type(" << type
                << ") of BaseInfoDriver! "
                        "Will use default sqlite3 BaseInfoDriver! "
                        "[DataDriverFactory::getBaseInfoDriver]");
        iter = m_baseInfoDrivers.find("SQLITE3");
        if (iter != m_baseInfoDrivers.end()) {
            result = m_baseInfoDrivers["SQLITE3"];
        } else {
            result = make_shared<SQLiteBaseInfoDriver>();
            m_baseInfoDrivers["SQLITE3"] = result;
        }
    }

    result->init(params);
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
    } else {
        HKU_INFO("Can't get the type(" << name
                << ") of BlockInfoDriver! "
                        "Will use default qianlong BaseInfoDriver! "
                        "[DataDriverFactory::getBlockDriver]");
        iter = m_blockDrivers.find("QIANLONG");
        if (iter != m_blockDrivers.end()) {
            result = iter->second;
        } else {
            result = make_shared<QLBlockInfoDriver>();
            m_blockDrivers["QIANLONG"] = result;
        }
    }

    result->init(params);
    return result;
}


} /* namespace hku */
