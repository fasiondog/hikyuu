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

/**
 * 新增用户
 */
class AddUserHandle : public RestHandle {
    REST_HANDLE_IMP(AddUserHandle)
    virtual void run() override {
        auto con = DB::getConnect();
        UserModel admin;
        con->load(admin, fmt::format("user_id={}", getCurrentUserId()));
        HTTP_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                   "No operation permission");

        check_missing_param("user");
        check_missing_param("password");
        UserModel user;
        user.setName(req["user"].get<string>());
        user.setPassword(req["password"].get<string>());
        user.setStartTime(Datetime::now());
        user.setStatus(UserModel::STATUS::NORMAL);
        {
            TransAction trans(con);
            int count = con->queryInt(fmt::format(R"(select count(id) from {} where name="{}")",
                                                  UserModel::getTableName(), user.getName()));
            HTTP_CHECK(count == 0, UserErrorCode::USER_NAME_REPETITION, "Duplicate user name");
            user.setUserId(DB::getNewUserId());
            con->save(user, false);
        }

        res["result"] = true;
        res["userid"] = user.getUserId();
        res["name"] = user.getName();
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
        HTTP_CHECK(admin.getName() == "admin", UserErrorCode::USER_NO_RIGHT,
                   "No operation permission");

        check_missing_param("user");
        {
            UserModel user;
            TransAction trans(con);
            con->load(user, fmt::format(R"(name="{}")", req["user"].get<std::string>()));
            if (user.id() != 0) {
                con->remove(user, false);
            }
        }

        res["result"] = true;
    }
};

}  // namespace hku