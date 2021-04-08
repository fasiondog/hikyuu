/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include <memory>
#include <hikyuu/utilities/ConnectPool.h>
#include <hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h>
#include <hikyuu/utilities/db_connect/mysql/MySQLConnect.h>
#include "TradeServiceDB.h"

namespace hku {

static std::unique_ptr<ConnectPool<SQLiteConnect>> g_trade_service_sqlite_pool;
static std::unique_ptr<ConnectPool<MySQLConnect>> g_trade_service_mysql_pool;

static void initTradeServiceSqlite(const Parameter& param) {}
static void initTradeServiceMysql(const Parameter& param) {}

void initTradeServiceDB(const Parameter& params) {
    string type = params.get<string>("type");
    if (type == "sqlite") {
        initTradeServiceSqlite(params);
    } else if (type == "mysql") {
        initTradeServiceMysql(params);
    } else {
        throw std::invalid_argument(fmt::format("Invalid database type: {}", type));
    }
}

DBConnectPtr getDBConnect() {
    return g_trade_service_sqlite_pool  ? g_trade_service_sqlite_pool->getAndWait()
           : g_trade_service_mysql_pool ? g_trade_service_mysql_pool->getAndWait()
                                        : DBConnectPtr();
}

}  // namespace hku