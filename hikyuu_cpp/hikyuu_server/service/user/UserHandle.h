/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-25
 *     Author: fasiondog
 */

#pragma once

#include <vector>
#include <hikyuu/utilities/db_connect/DBCondition.h>
#include "common/uuid.h"
#include "../TokenCache.h"
#include "../RestHandle.h"
#include "model/UserModel.h"
#include "model/TokenModel.h"

namespace hku {

/***********************************
 *
 * 用户相关操作仅有 admin 有权限
 *
 **********************************/

/**
 * 新增用户
 */
class AddUserHandle : public RestHandle {
public:
    static const int MIN_NAME_LENGTH = 1;
    static const int MAX_NAME_LENGTH = 128;
    static const int MAX_PASSWORD_LENGTH = 72;  // 目前 bcrypt 最长密码只支持到72位

    REST_HANDLE_IMP(AddUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("userid={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        user.setName(req["user"].get<string>());
        size_t name_len = user.getName().size();
        REQ_CHECK(name_len >= MIN_NAME_LENGTH && name_len <= MAX_NAME_LENGTH,
                  UserErrorCode::USER_INVALID_NAME_OR_PASSWORD,
                  _ctr("user", "The user name must be 1 to 128 characters long"));

        std::string pwd = req["password"].get<string>();
        size_t password_len = pwd.size();
        REQ_CHECK(password_len <= MAX_PASSWORD_LENGTH, UserErrorCode::USER_INVALID_NAME_OR_PASSWORD,
                  fmt::format(_ctr("user", "The password must be less than {} characters"),
                              MAX_PASSWORD_LENGTH));

        user.setPassword(req["password"].get<string>());
        user.setStartTime(Datetime::now());
        user.setStatus(UserModel::STATUS::NORMAL);
        {
            TransAction trans(con);
            int count = con->queryInt(fmt::format(R"(select count(id) from {} where name="{}")",
                                                  UserModel::getTableName(), user.getName()));
            REQ_CHECK(count == 0, UserErrorCode::USER_NAME_REPETITION,
                      _ctr("user", "Unavailable user name"));
            user.setUserId(DB::getNewUserId());
            con->save(user, false);
        }

        res["userid"] = user.getUserId();
        res["name"] = user.getName();
        res["start_time"] = user.getStartTime().str();
    }
};

/**
 * 删除用户
 */
class RemoveUserHandle : public RestHandle {
    REST_HANDLE_IMP(RemoveUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("userid={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("userid");
        uint64_t userid = req["userid"].get<uint64_t>();
        REQ_CHECK(userid != admin.getUserId(), UserErrorCode::USER_TRY_DELETE_ADMIN,
                  _ctr("user", "The admin account cannot be deleted"));
        {
            UserModel user;
            TransAction trans(con);
            con->load(user, fmt::format(R"(userid="{}")", req["userid"].get<uint64_t>()));
            if (user.id() != 0 && user.getStatus() != UserModel::DELETED) {
                TokenModel tm;
                con->load(tm, DBCondition(Field("userid") == user.getUserId()));
                if (tm.id() != 0) {
                    con->remove(tm, false);
                    TokenCache::remove(tm.getToken());
                }
                user.setEndTime(Datetime::now());
                user.setStatus(UserModel::DELETED);
                con->save(user, false);
            }
        }
    }
};

class QueryUserHandle : public RestHandle {
    REST_HANDLE_IMP(QueryUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("userid={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        DBCondition cond;
        QueryParams params;
        if (getQueryParams(params)) {
            auto iter = params.find("userid");
            if (iter != params.end()) {
                cond = Field("userid") == (uint64_t)std::stoll(iter->second);
            }

            iter = params.find("name");
            if (iter != params.end()) {
                cond = cond & (Field("name") == iter->second);
            }
        }

        cond = (cond & (Field("status") != UserModel::DELETED)) + ASC("start_time");

        std::vector<UserModel> users;
        con->batchLoad(users, cond.str());
        json jarray;
        for (auto& user : users) {
            json j;
            j["userid"] = user.getUserId();
            j["name"] = user.getName();
            j["start_time"] = user.getStartTime().str();
            jarray.push_back(j);
        }
        res["data"] = jarray;
    }
};

class ResetPasswordUserHandle : public RestHandle {
    REST_HANDLE_IMP(ResetPasswordUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("userid={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("userid");
        {
            UserModel user;
            TransAction trans(con);
            con->load(user, fmt::format(R"(userid="{}")", req["userid"].get<uint64_t>()));
            if (user.id() != 0 && user.getStatus() != UserModel::DELETED) {
                user.setPassword(
                  "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");  // 123456
                con->save(user, false);
            }
        }
    }
};

class ChangePasswordUserHandle : public RestHandle {
    REST_HANDLE_IMP(ChangePasswordUserHandle)
    virtual void run() override {
        check_missing_param({"old", "new", "confirm"});
        uint64_t userid = getCurrentUserId();
        UserModel user;
        auto con = DB::getConnect();
        con->load(user, Field("userid") == userid);
        REQ_CHECK(user.checkPassword(req["old"].get<std::string>()),
                  UserErrorCode::USER_INVALID_NAME_OR_PASSWORD, "old password error!");
        std::string new_pwd = req["new"].get<std::string>();
        REQ_CHECK(new_pwd == req["confirm"].get<std::string>(),
                  UserErrorCode::USER_INVALID_NAME_OR_PASSWORD, "The two passwords are different!");
        user.setPassword(new_pwd);
        con->save(user);
    }
};

}  // namespace hku