/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-25
 *     Author: fasiondog
 */

#pragma once

#include "common/uuid.h"
#include "../TokenCache.h"
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
            AutoTransAction trans(con);
            int count = con->queryInt(fmt::format(R"(select count(id) from {} where name="{}")",
                                                  UserModel::getTableName(), user.getName()), 0);
            REQ_CHECK(count == 0, UserErrorCode::USER_NAME_REPETITION,
                      _ctr("user", "Unavailable user name"));
            user.setUserId(DB::getNewUserId());
            con->save(user, false);
        }

        TokenModel token;
        token.setUserId(user.getUserId());
        token.setToken(createToken(user.getUserId()));
        con->save(token);

        TokenCache::put(token.getToken());
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
        con->load(user, DBCondition((Field("name") == std::string(req["user"].get<std::string>())) &
                                    (Field("status") != UserModel::DELETED)));
        REQ_CHECK(user.id() != 0, UserErrorCode::USER_NOT_EXIST,
                  _ctr("user", "User does not exist"));
        REQ_CHECK(user.checkPassword(req["password"].get<std::string>()),
                  UserErrorCode::USER_INVALID_NAME_OR_PASSWORD, _ctr("user", "Wrong password"));

        TokenModel token;
        {
            con->load(token, fmt::format(R"(userid={})", user.getUserId()));
            if (token.id() == 0) {
                token.setUserId(user.getUserId());
            }

            // 总是生成新的 token
            token.setToken(createToken(user.getUserId()));
            con->save(token);
        }
        TokenCache::put(token.getToken());
        res["userid"] = user.getUserId();
        res["token"] = token.getToken();
        res["result"] = true;
    }
};

// 退出登录
class LogoutHandle : public RestHandle {
    REST_HANDLE_IMP(LogoutHandle)
    virtual void run() override {
        std::string token = getReqHeader("hku_token");
        if (TokenCache::have(token)) {
            TokenCache::remove(token);
        }
        try {
            auto con = DB::getConnect();
            AutoTransAction trans(con);
            con->exec(
              fmt::format(R"(delete from {} where token="{}")", TokenModel::getTableName(), token));
        } catch (std::exception& e) {
            CLS_ERROR("failed delete token from database. {}", e.what());
        } catch (...) {
            CLS_ERROR("Unknown error!");
        }
    }
};

}  // namespace hku