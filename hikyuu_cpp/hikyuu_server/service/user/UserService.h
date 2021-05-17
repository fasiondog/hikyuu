/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "db/db.h"
#include "sql/sqlite/sqlitedb.h"
#include "SignHandle.h"
#include "UserHandle.h"

namespace hku {

class UserService : public HttpService {
public:
    UserService(const char *url) : HttpService(url) {
        auto con = DB::getConnect();
        if (DB::isSQLite()) {
            DBUpgrade(con, "admin", {}, 2, g_sqlite_create_usr_db);
        }
    }

    virtual void regHandle() override {
        POST<SignupHandle>("signup");
        POST<LoginHandle>("login");
        POST<LogoutHandle>("logout");

        POST<AddUserHandle>("user");
        DEL<RemoveUserHandle>("user");
        GET<QueryUserHandle>("user");

        PUT<ResetPasswordUserHandle>("reset_password");
    }
};

}  // namespace hku