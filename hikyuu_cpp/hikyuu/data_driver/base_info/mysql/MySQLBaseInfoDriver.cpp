/*
 * MySQLBaseInfoDriver.cpp
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "MySQLBaseInfoDriver.h"

#include "../../../utilities/util.h"
#include "../../../StockManager.h"
#include "../../../Log.h"
#include "../table/MarketInfoTable.h"
#include "../table/StockTypeInfoTable.h"
#include "../table/StockWeightTable.h"
#include "../table/StockTable.h"

namespace hku {

MySQLBaseInfoDriver::MySQLBaseInfoDriver() : BaseInfoDriver("mysql"), m_pool(nullptr) {}

MySQLBaseInfoDriver::~MySQLBaseInfoDriver() {
    if (m_pool) {
        delete m_pool;
    }
}

bool MySQLBaseInfoDriver::_init() {
    HKU_ASSERT_M(m_pool == nullptr, "Maybe repeat initialization!");
    Parameter connect_param;
    connect_param.set<string>("host", getParamFromOther<string>(m_params, "host", "127.0.0.1"));
    connect_param.set<string>("usr", getParamFromOther<string>(m_params, "usr", "root"));
    connect_param.set<string>("pwd", getParamFromOther<string>(m_params, "pwd", ""));
    connect_param.set<string>("db", getParamFromOther<string>(m_params, "db", "hku_base"));
    string port_str = getParamFromOther<string>(m_params, "port", "3306");
    unsigned int port = boost::lexical_cast<unsigned int>(port_str);
    connect_param.set<int>("port", port);
    m_pool = new ConnectPool<MySQLConnect>(connect_param);
    return true;
}

bool MySQLBaseInfoDriver::_loadMarketInfo() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    auto con = m_pool->getConnect();

    vector<MarketInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception &e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    StockManager &sm = StockManager::instance();
    for (auto &info : infoTables) {
        Datetime lastDate;
        try {
            lastDate = Datetime(info.lastDate() * 10000);
        } catch (...) {
            lastDate = Null<Datetime>();
            HKU_WARN("lastDate of market({}) is invalid! ", info.market());
        }
        sm.loadMarketInfo(
          MarketInfo(info.market(), info.name(), info.description(), info.code(), lastDate));
    }

    return true;
}

bool MySQLBaseInfoDriver::_loadStockTypeInfo() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    auto con = m_pool->getConnect();

    vector<StockTypeInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception &e) {
        HKU_FATAL("load StockTypeInfo table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load StockTypeInfo table failed!");
        return false;
    }

    StockManager &sm = StockManager::instance();
    for (auto &info : infoTables) {
        sm.loadStockTypeInfo(StockTypeInfo(info.type(), info.description(), info.tick(),
                                           info.tickValue(), info.precision(),
                                           info.minTradeNumber(), info.maxTradeNumber()));
    }

    return true;
}

StockWeightList MySQLBaseInfoDriver::_getStockWeightList(uint64_t stockid) {
    StockWeightList result;
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return result;
    }

    auto con = m_pool->getConnect();
    HKU_ASSERT(con);

    vector<StockWeightTable> table;
    try {
        con->batchLoad(table, format("stockid={}", stockid));
    } catch (std::exception &e) {
        HKU_FATAL("load StockWeight table failed! {}", e.what());
        return result;
    } catch (...) {
        HKU_FATAL("load StockWeight table failed!");
        return result;
    }

    for (auto &w : table) {
        try {
            result.push_back(StockWeight(Datetime(w.date * 10000), w.countAsGift * 0.0001,
                                         w.countForSell * 0.0001, w.priceForSell * 0.001,
                                         w.bonus * 0.001, w.countOfIncreasement * 0.0001,
                                         w.totalCount, w.freeCount));
        } catch (std::out_of_range &e) {
            HKU_WARN("Date of id({}) is invalid! {}", w.id(), e.what());
        } catch (std::exception &e) {
            HKU_WARN("Error StockWeight Record id({}) {}", w.id(), e.what());
        } catch (...) {
            HKU_WARN("Error StockWeight Record id({})! Unknow reason!", w.id());
        }
    }

    return result;
}

bool MySQLBaseInfoDriver::_loadStock() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    auto con = m_pool->getConnect();

    vector<MarketInfoTable> marketTable;
    try {
        con->batchLoad(marketTable);
    } catch (std::exception &e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    unordered_map<uint64_t, string> marketDict;
    for (auto &m : marketTable) {
        marketDict[m.id()] = m.market();
    }

    vector<StockTable> table;
    try {
        con->batchLoad(table);
    } catch (std::exception &e) {
        HKU_FATAL("load Stock table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Stock table failed!");
        return false;
    }

    Stock stock;
    StockTypeInfo stockTypeInfo;
    StockTypeInfo null_stockTypeInfo;
    StockManager &sm = StockManager::instance();
    for (auto &r : table) {
        Datetime startDate, endDate;
        if (r.startDate > r.endDate || r.startDate == 0 || r.endDate == 0) {
            //日期非法，置为Null<Datetime>
            startDate = Null<Datetime>();
            endDate = Null<Datetime>();
        } else {
            startDate =
              (r.startDate == 99999999LL) ? Null<Datetime>() : Datetime(r.startDate * 10000);
            endDate = (r.endDate == 99999999LL) ? Null<Datetime>() : Datetime(r.endDate * 10000);
        }

        stockTypeInfo = sm.getStockTypeInfo(r.type);
        if (stockTypeInfo != null_stockTypeInfo) {
            stock =
              Stock(marketDict[r.marketid], r.code, r.name, r.type, r.valid, startDate, endDate,
                    stockTypeInfo.tick(), stockTypeInfo.tickValue(), stockTypeInfo.precision(),
                    stockTypeInfo.minTradeNumber(), stockTypeInfo.maxTradeNumber());

        } else {
            stock =
              Stock(marketDict[r.marketid], r.code, r.name, r.type, r.valid, startDate, endDate);
        }

        if (sm.loadStock(stock)) {
            StockWeightList weightList = _getStockWeightList(r.stockid);
            stock.setWeightList(weightList);
        }
    }

    return true;
}

} /* namespace hku */
