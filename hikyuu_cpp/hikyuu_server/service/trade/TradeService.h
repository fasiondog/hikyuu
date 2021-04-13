/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include <memory>
#include <hikyuu/utilities/ConnectPool.h>
#include <hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h>
#include <hikyuu/utilities/db_connect/mysql/MySQLConnect.h>
#include "http/HttpService.h"
#include "common/snowflake.h"
#include "WalletHandle.h"
#include "TradeAccountHandle.h"

namespace hku {

class TradeService : public HttpService {
    CLASS_LOGGER(TradeService)

public:
    TradeService() = delete;
    TradeService(const char *url) = delete;

    TradeService(const char *url, const std::string &config_file);

    virtual void regHandle() override {
        GET<WalletHandle>("wallet");

        POST<AddTradeAccountHandle>("account");
        GET<GetTradeAccountHandle>("account");
        PUT<ModTradeAccountHandle>("account");
        DEL<DelTradeAccountHandle>("account");
    }

public:
    static DBConnectPtr getDBConnect();

    static int64_t newTdId() {
        return ms_td_id_generator.nextid();
    }

    static int64_t newStaId() {
        return ms_sta_id_generator.nextid();
    }

    static bool isValidEumValue(const std::string &table, const std::string &field,
                                const std::string &val);

private:
    static void initTradeServiceSqlite(const Parameter &param);
    static void initTradeServiceMysql(const Parameter &param);
    static void initTradeServiceDB(const Parameter &params);

private:
    static std::unique_ptr<ConnectPool<SQLiteConnect>> ms_sqlite_pool;
    static std::unique_ptr<ConnectPool<MySQLConnect>> ms_mysql_pool;

    using snowflake_t = snowflake<1618243200000L, std::mutex>;
    static snowflake_t ms_td_id_generator;
    static snowflake_t ms_sta_id_generator;
};

}  // namespace hku