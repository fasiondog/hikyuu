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
#include "../table/StockTypeInfoTable.h"
#include "../table/StockWeightTable.h"
#include "../table/StockTable.h"

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
        HKU_ERROR("Connect pool ptr is null!");
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
        sm.loadMarketInfo(
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

bool SQLiteBaseInfoDriverV2::_loadStockTypeInfo() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<StockTypeInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception& e) {
        HKU_FATAL("load StockTypeInfo table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load StockTypeInfo table failed!");
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto& info: infoTables) {
        sm.loadStockTypeInfo(
            StockTypeInfo(
                info.type(),
                info.description(), 
                info.tick(),
                info.tickValue(), 
                info.precision(),
                info.minTradeNumber(), 
                info.maxTradeNumber()
        ));
    }

    return true;
}

bool SQLiteBaseInfoDriverV2::_loadStock() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<MarketInfoTable> marketTable;
    try {
        con->batchLoad(marketTable);
    } catch (std::exception& e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    unordered_map<uint64, string> marketDict;
    for (auto& m: marketTable) {
        marketDict[m.id()] = m.name();
    }

    vector<StockTable> table;
    try {
        con->batchLoad(table);
    } catch (std::exception& e) {
        HKU_FATAL("load Stock table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Stock table failed!");
        return false;
    }

    Stock stock;
    StockTypeInfo stockTypeInfo;
    StockTypeInfo null_stockTypeInfo;
    StockManager& sm = StockManager::instance();
    for (auto& r: table) {
        Datetime startDate, endDate;
        if(r.startDate > r.endDate || r.startDate == 0 || r.endDate == 0) {
            //日期非法，置为Null<Datetime>
            startDate = Null<Datetime>();
            endDate = Null<Datetime>();
        } else {
            startDate = (r.startDate == 99999999LL)
                      ? Null<Datetime>()
                      : Datetime(r.startDate*10000);
            endDate = (r.endDate == 99999999LL)
                    ? Null<Datetime>()
                    : Datetime(r.endDate*10000);
        }

        stockTypeInfo = sm.getStockTypeInfo(r.type);
        if(stockTypeInfo != null_stockTypeInfo) {
            stock = Stock(marketDict[r.marketid],
                          r.code,
                          r.name,
                          r.type,
                          r.valid,
                          startDate,
                          endDate,
                          stockTypeInfo.tick(),
                          stockTypeInfo.tickValue(),
                          stockTypeInfo.precision(),
                          stockTypeInfo.minTradeNumber(),
                          stockTypeInfo.maxTradeNumber());
            
        } else {
            stock = Stock(marketDict[r.marketid],
                          r.code,
                          r.name,
                          r.type,
                          r.valid,
                          startDate,
                          endDate);
        }

        if(sm.loadStock(stock)){
            StockWeightList weightList = _getStockWeightList(r.stockid);
            stock.setWeightList(weightList);
        }
    }

    return true;
}

StockWeightList SQLiteBaseInfoDriverV2::_getStockWeightList(uint64 stockid) {
    StockWeightList result;
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return result;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<StockWeightTable> table;
    try {
        con->batchLoad(table, format("stockid={}", stockid));
    } catch (std::exception& e) {
        HKU_FATAL("load StockWeight table failed! {}", e.what());
        return result;
    } catch (...) {
        HKU_FATAL("load StockWeight table failed!");
        return result;
    }

    for (auto& w: table) {
        try {
            result.push_back(
                StockWeight(
                    Datetime(w.date*10000), 
                    w.countAsGift * 0.0001,
                    w.countForSell * 0.0001, 
                    w.priceForSell * 0.001,
                    w.bonus * 0.001,
                    w.countOfIncreasement * 0.0001,
                    w.totalCount, 
                    w.freeCount
                )
            );
        } catch (std::out_of_range& e) {
            HKU_WARN("Date of id({}) is invalid! {}", w.id(), e.what());
        } catch (std::exception& e) {
            HKU_WARN("Error StockWeight Record id({}) {}", w.id(), e.what());
        } catch (...) {
            HKU_WARN("Error StockWeight Record id({})! Unknow reason!", w.id());
        }
    }

    return result;
}

} /* namespace */