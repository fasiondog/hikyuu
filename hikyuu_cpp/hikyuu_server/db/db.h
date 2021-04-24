/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-24
 *     Author: fasiondog
 */

#pragma once

#include <memory>
#include <hikyuu/utilities/ConnectPool.h>
#include <hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h>
#include <hikyuu/utilities/db_connect/mysql/MySQLConnect.h>
#include "../common/snowflake.h"

namespace hku {

class DB {
public:
    static void init(const std::string &config_file);

    static DBConnectPtr getConnect();

    static bool isValidEumValue(const std::string &table, const std::string &field,
                                const std::string &val);

    static int64_t getNewTdId() {
        return ms_td_id_generator.nextid();
    }

    static int64_t getNewStaId() {
        return ms_sta_id_generator.nextid();
    }

private:
    static void initSqlite(const Parameter &param);
    static void initMysql(const Parameter &param);
    static void initDB(const Parameter &params);

private:
    static std::unique_ptr<ConnectPool<SQLiteConnect>> ms_sqlite_pool;
    static std::unique_ptr<ConnectPool<MySQLConnect>> ms_mysql_pool;

    using snowflake_t = snowflake<1618243200000L, std::mutex>;
    static snowflake_t ms_td_id_generator;
    static snowflake_t ms_sta_id_generator;
};

}  // namespace hku