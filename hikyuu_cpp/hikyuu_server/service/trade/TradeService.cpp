/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-09
 *     Author: fasiondog
 */

#include <hikyuu/utilities/IniParser.h>
#include <hikyuu/utilities/db_connect/DBUpgrade.h>
#include "TradeService.h"
#include "db/sqlite/sqlitedb.h"

namespace hku {

std::unique_ptr<ConnectPool<SQLiteConnect>> TradeService::ms_sqlite_pool;
std::unique_ptr<ConnectPool<MySQLConnect>> TradeService::ms_mysql_pool;

void TradeService::initTradeServiceSqlite(const Parameter& param) {
    Parameter sqlite_param;
    sqlite_param.set<string>("db", param.get<string>("db"));
    sqlite_param.set<int>("flags",
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
    ms_sqlite_pool = std::make_unique<ConnectPool<SQLiteConnect>>(sqlite_param);
    auto con = ms_sqlite_pool->getAndWait();
    DBUpgrade(con, "td", {}, 2, g_sqlite_create_db);
}

void TradeService::initTradeServiceMysql(const Parameter& param) {
    Parameter mysql_param;
    mysql_param.set<string>("host", param.get<string>("host"));
    mysql_param.set<string>("usr", param.get<string>("usr"));
    mysql_param.set<string>("pwd", param.get<string>("pwd"));
    mysql_param.set<string>("db", param.get<string>("db"));
    mysql_param.set<int>("port", std::stoi(param.get<string>("port")));
    ms_mysql_pool = std::make_unique<ConnectPool<MySQLConnect>>(mysql_param);
}

void TradeService::initTradeServiceDB(const Parameter& params) {
    CLS_TRACE("Initialize trade service database");
    string type = params.get<string>("type");
    if (type == "sqlite3") {
        CLS_TRACE("use sqlite3");
        initTradeServiceSqlite(params);
    } else if (type == "mysql") {
        CLS_TRACE("use mysql");
        initTradeServiceMysql(params);
    } else {
        throw std::invalid_argument(fmt::format("Invalid database type: {}", type));
    }
}

DBConnectPtr TradeService::getDBConnect() {
    return ms_sqlite_pool  ? ms_sqlite_pool->getAndWait()
           : ms_mysql_pool ? ms_mysql_pool->getAndWait()
                           : DBConnectPtr();
}

TradeService::TradeService(const char* url, const std::string& config_file) : HttpService(url) {
    if (ms_sqlite_pool || ms_mysql_pool) {
        return;
    }

    IniParser ini;
    ini.read(config_file);

    Parameter param;
    auto options = ini.getOptionList("database");
    for (auto& option : *options) {
        param.set<string>(option, ini.get("database", option));
    }

    initTradeServiceDB(param);
}

}  // namespace hku