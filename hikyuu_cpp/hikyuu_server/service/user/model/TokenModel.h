/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-26
 *     Author: fasiondog
 */

#pragma once

#include <string_view>
#include <hikyuu/utilities/db_connect/TableMacro.h>

namespace hku {

class TokenModel {
    TABLE_BIND3(admin_token, token, create_time, expire_time)

public:
    TokenModel() : create_time(0), expire_time(0) {}

    string getToken() const {
        return token;
    }

    void setToken(const string& token) {
        this->token = token;
    }

    Datetime getCreateTime() const {
        return Datetime(create_time);
    }

    void setCreatetime(Datetime time) {
        create_time = time.hex();
    }

    Datetime getExpireTime() const {
        return Datetime(expire_time);
    }

    void setExpireTime(Datetime time) {
        expire_time = time.hex();
    }

private:
    string token;
    uint64_t create_time;  // 创建时间，精确到秒
    uint64_t expire_time;  // 过期时间，精确到秒
};

}  // namespace hku