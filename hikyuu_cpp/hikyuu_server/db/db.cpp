/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-24
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/utilities/IniParser.h>
#include "../common/log.h"
#include "db.h"

namespace hku {

std::unique_ptr<ConnectPool<SQLiteConnect>> DB::ms_sqlite_pool;
std::unique_ptr<ConnectPool<MySQLConnect>> DB::ms_mysql_pool;

DB::snowflake_t DB::ms_userid_generator;
DB::snowflake_t DB::ms_td_id_generator;
DB::snowflake_t DB::ms_sta_id_generator;

void DB::init(const std::string& config_file) {
    if (ms_sqlite_pool || ms_mysql_pool) {
        return;
    }

    ms_userid_generator.init(1, 1);
    ms_td_id_generator.init(1, 1);
    ms_sta_id_generator.init(1, 1);

    IniParser ini;
    ini.read(config_file);

    Parameter param;
    auto options = ini.getOptionList("database");
    for (auto& option : *options) {
        param.set<string>(option, ini.get("database", option));
    }

    initDB(param);
}

void DB::initSqlite(const Parameter& param) {
    Parameter sqlite_param;
    sqlite_param.set<string>("db", param.get<string>("db"));
    sqlite_param.set<int>("flags",
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
    ms_sqlite_pool = std::make_unique<ConnectPool<SQLiteConnect>>(sqlite_param);
}

void DB::initMysql(const Parameter& param) {
    Parameter mysql_param;
    mysql_param.set<string>("host", param.get<string>("host"));
    mysql_param.set<string>("usr", param.get<string>("usr"));
    mysql_param.set<string>("pwd", param.get<string>("pwd"));
    mysql_param.set<string>("db", param.get<string>("db"));
    mysql_param.set<int>("port", std::stoi(param.get<string>("port")));
    ms_mysql_pool = std::make_unique<ConnectPool<MySQLConnect>>(mysql_param);
}

void DB::initDB(const Parameter& params) {
    string type = params.get<string>("type");
    if (type == "sqlite3") {
        APP_TRACE("use sqlite3 database");
        initSqlite(params);
    } else if (type == "mysql") {
        APP_TRACE("use mysql database");
        initMysql(params);
    } else {
        throw std::invalid_argument(fmt::format("Invalid database type: {}", type));
    }
}

DBConnectPtr DB::getConnect() {
    return ms_sqlite_pool  ? ms_sqlite_pool->getAndWait()
           : ms_mysql_pool ? ms_mysql_pool->getAndWait()
                           : DBConnectPtr();
}

bool DB::isValidEumValue(const std::string& table, const std::string& field,
                         const std::string& val) {
    int count = getConnect()->queryInt(
      fmt::format(
        R"(select count(1) from td_enum where table_name="{}" and field_name="{}" and value="{}")",
        table, field, val),
      0);
    return count > 0;
}

}  // namespace hku