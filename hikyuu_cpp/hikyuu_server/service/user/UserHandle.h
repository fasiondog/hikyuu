/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-25
 *     Author: fasiondog
 */

#pragma once

#include "common/uuid.h"
#include "../RestHandle.h"
#include "model/UserModel.h"
#include "model/TokenModel.h"

namespace hku {

// 用户注册
class SignupHandle : public NoAuthRestHandle {
    NO_AUTH_REST_HANDLE_IMP(SignupHandle)
    virtual void run() override {
        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        user.setName(req["user"].get<string>());
        user.setPassword(req["password"].get<string>());
        user.setStartTime(Datetime::now());
        user.setStatus(UserModel::STATUS::NORMAL);
        auto con = DB::getConnect();
        {
            TransAction trans(con);
            int count = con->queryInt(fmt::format(R"(select count(id) from {} where name="{}")",
                                                  UserModel::getTableName(), user.getName()));
            HTTP_CHECK(count == 0, UserErrorCode::USER_NAME_REPETITION, "Duplicate user name");
            user.setUserId(DB::getNewUserId());
            con->save(user, false);
        }

        TokenModel token;
        token.setToken(UUID());
        Datetime now = Datetime::now();
        token.setCreateTime(now);
        token.setExpireTime(now + TimeDelta(30));
        con->save(token);

        TokenCache::add(token.getToken(), token.getExpireTime());
        res["userid"] = user.getUserId();
        res["token"] = token.getToken();
        res["result"] = true;
    }
};

// 用户登录
class LoginHandle : public NoAuthRestHandle {
    NO_AUTH_REST_HANDLE_IMP(LoginHandle)

    virtual void run() override {
        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        auto con = DB::getConnect();
        con->load(user, fmt::format(R"(name="{}")", req["user"].get<std::string>()));
        HTTP_CHECK(user.id() != 0, UserErrorCode::USER_NOT_EXIST, "User does not exist");
        HTTP_CHECK(user.getPassword() == req["password"].get<std::string>(),
                   UserErrorCode::USER_WRONG_PASSWORD, "Wrong password");
    }
};

// 退出登录
class LogoutHandle : public HttpHandle {
    HTTP_HANDLE_IMP(LogoutHandle)

    virtual void run() override {
        setResHeader("Content-Type", "application/json; charset=UTF-8");
        setResData(R"({"result": true})");
    }
};

}  // namespace hku