/*
 * SQLiteBaseInfoDriver.cpp
 *
 *  Copyright (c) 2019 fasiondog
 * 
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include <thread>
#include "../../../StockManager.h"
#include "SQLiteBaseInfoDriverV2.h"
#include "../table/MarketInfoTable.h"

namespace hku {

SQLiteBaseInfoDriverV2::SQLiteBaseInfoDriverV2()
: BaseInfoDriver("sqlite3"), m_pool(nullptr) {
}

SQLiteBaseInfoDriverV2::~SQLiteBaseInfoDriverV2() {
    if (m_pool) {
        delete m_pool;
    }
}

bool SQLiteBaseInfoDriverV2::_init() {
    string dbname;
    try {
        dbname = getParam<string>("db");
        HKU_TRACE("SQLITE3: {}", dbname);
    } catch(...) {
        HKU_ERROR("Can't get Sqlite3 filename!");
        return false;
    }

    int cpu_num = std::thread::hardware_concurrency();
    if (cpu_num > 1) {
        cpu_num--;
    }
    m_pool = new DBConnectPool(m_params, cpu_num);
    return true;
}

bool SQLiteBaseInfoDriverV2::_loadMarketInfo() {
    if (!m_pool) {
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();
    
    vector<MarketInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception& e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto& info: infoTables) {
        Datetime lastDate;
        try {
            lastDate = Datetime(info.lastDate() * 10000);
        } catch (...) {
            lastDate = Null<Datetime>();
            HKU_WARN("lastDate of market({}) is invalid! ", info.market());
        }
        sm.addMarketInfo(
            MarketInfo(
                info.market(),
                info.name(),
                info.description(),
                info.code(),
                lastDate
        ));
    }

    return true;
}

} /* namespace */