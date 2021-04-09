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
#include "WalletHandle.h"

namespace hku {

class TradeService : public HttpService {
    CLASS_LOGGER(TradeService)

public:
    TradeService() = delete;
    TradeService(const char *url) = delete;

    TradeService(const char *url, const std::string &config_file);

    virtual void regHandle() override {
        GET<WalletHandle>("wallet");
    }

public:
    DBConnectPtr getDBConnect();

private:
    static void initTradeServiceSqlite(const Parameter &param);
    static void initTradeServiceMysql(const Parameter &param);
    static void initTradeServiceDB(const Parameter &params);

private:
    static std::unique_ptr<ConnectPool<SQLiteConnect>> ms_sqlite_pool;
    static std::unique_ptr<ConnectPool<MySQLConnect>> ms_mysql_pool;
};

}  // namespace hku