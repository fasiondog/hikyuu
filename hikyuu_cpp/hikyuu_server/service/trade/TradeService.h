/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "sql/sqlite/sqlitedb.h"
#include "WalletHandle.h"
#include "TradeAccountHandle.h"
#include "XueqiuAccountHandle.h"

namespace hku {

class TradeService : public HttpService {
    CLASS_LOGGER(TradeService)

public:
    TradeService() = delete;
    TradeService(const char* url) : HttpService(url) {
        auto con = DB::getConnect();
        if (DB::isSQLite()) {
            DBUpgrade(con, "td", {}, 2, g_sqlite_create_td_db);
        }
    }

    virtual void regHandle() override {
        GET<QueryAllXueqiuAccountHandle>("xqaccounts");
        GET<QueryXueqiuAccountHandle>("xqaccount");
        POST<AddXueqiuAccountHandle>("xqaccount");
        DEL<RemoveXueqiuAccountHandle>("xqaccount");
        PUT<ModifyXueqiuAccountHandle>("xqaccount");

        GET<WalletHandle>("wallet");
        POST<AddTradeAccountHandle>("account");
        GET<GetTradeAccountHandle>("account");
        PUT<ModTradeAccountHandle>("account");
        DEL<DelTradeAccountHandle>("account");
    }
};

}  // namespace hku