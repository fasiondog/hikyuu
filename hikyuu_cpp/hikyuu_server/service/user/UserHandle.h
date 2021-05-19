/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-25
 *     Author: fasiondog
 */

#pragma once

#include <vector>
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
    static const int MAX_PASSWORD_LENGTH = 16;

    REST_HANDLE_IMP(AddUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("user_id={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        user.setName(req["user"].get<string>());
        size_t name_len = user.getName().size();
        REQ_CHECK(name_len >= MIN_NAME_LENGTH && name_len <= MAX_NAME_LENGTH,
                  UserErrorCode::USER_INVALID_NAME,
                  _ctr("user", "The user name must be 1 to 128 characters long"));

        user.setPassword(req["password"].get<string>());
        size_t password_len = user.getPassword().size();
        REQ_CHECK(password_len <= MAX_PASSWORD_LENGTH, UserErrorCode::USER_INVALID_PASSWORD,
                  fmt::format(_ctr("user", "The password must be less than {} characters"),
                              MAX_PASSWORD_LENGTH));

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
        con->load(admin, fmt::format("user_id={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("userid");
        {
            UserModel user;
            TransAction trans(con);
            con->load(user, fmt::format(R"(user_id="{}")", req["userid"].get<uint64_t>()));
            if (user.id() != 0 && user.getStatus() != UserModel::DELETED) {
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
        con->load(admin, fmt::format("user_id={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        std::vector<UserModel> users;
        con->batchLoad(users,
                       fmt::format("status<>{} order by start_time DESC", UserModel::DELETED));
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
        con->load(admin, fmt::format("user_id={}", getCurrentUserId()));
        REQ_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                  _ctr("user", "No operation permission"));

        check_missing_param("userid");
        {
            UserModel user;
            TransAction trans(con);
            con->load(user, fmt::format(R"(user_id="{}")", req["userid"].get<uint64_t>()));
            if (user.id() != 0 && user.getStatus() != UserModel::DELETED) {
                user.setPassword("123456");
                con->save(user, false);
            }
        }
    }
};

}  // namespace hku