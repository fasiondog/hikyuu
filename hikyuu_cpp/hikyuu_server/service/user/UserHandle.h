/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-25
 *     Author: fasiondog
 */

#pragma once

#include "../RestHandle.h"
#include "model/UserModel.h"
#include "model/TokenModel.h"

namespace hku {

class SignupHandle : public NoAuthRestHandle {
    NO_AUTH_REST_HANDLE_IMP(SignupHandle)
    virtual void run() override {
        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        user.setName(req["user"].get<string>());
        user.setPassword(req["password"].get<string>());
        user.setStartTime(Datetime::now());
        {
            auto con = DB::getConnect();
            TransAction trans(con);
            int count = con->queryInt(fmt::format(R"(select count(id) from {} where name="{}")",
                                                  UserModel::getTableName(), user.getName()));
            HTTP_CHECK(count == 0, UserErrorCode::USER_NAME_REPETITION, "Duplicate user name");
            user.setUserId(DB::getNewUserId());
            con->save(user, false);
        }
        setResData(
          R"({"hku_token":"7c98806c0711cf996d602890e0ab9119d9a86afe04296ba69a16f0d9d76be755"})");
    }
};

}  // namespace hku