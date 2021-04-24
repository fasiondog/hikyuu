/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-24
 *     Author: fasiondog
 */

#pragma once

#include <string_view>
#include <hikyuu/utilities/db_connect/TableMacro.h>

namespace hku {

class UserModel {
    TABLE_BIND6(user, user_id, name, password, create_time, token, expire_time)

public:
    int64_t getUserId() const {
        return user_id;
    }

    void setUserId(int64_t id) {
        user_id = id;
    }

    std::string getName() const {
        return name;
    }

    void setName(const std::string& name) {
        this->name = name;
    }

    std::string getPassword() const {
        return password;
    }

    void setPassword(const std::string& password) {
        this->password = password;
    }

    Datetime getCreateTime() const {
        return Datetime(create_time);
    }

    void setCreateTime(Datetime time) {
        create_time = time.number();
    }

    std::string getToken() const {
        return token;
    }

    void setToken(std::string_view token) {
        this->token = token;
    }

private:
    int64_t user_id;       // 用户id
    std::string name;      // 用户名
    std::string password;  // 用户密码
    uint64_t create_time;  // 用户创建时间
    std::string token;
    uint64_t expire_time;  // token 过期时间
};

}  // namespace hku