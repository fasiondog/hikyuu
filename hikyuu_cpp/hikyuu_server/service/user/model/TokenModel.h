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
    TABLE_BIND2(admin_token, user_id, token)

public:
    TokenModel() = default;

    uint64_t getUserId() const {
        return user_id;
    }

    void setUserId(uint64_t id) {
        user_id = id;
    }

    string getToken() const {
        return token;
    }

    void setToken(const string& token) {
        this->token = token;
    }

private:
    uint64_t user_id;
    string token;
};

}  // namespace hku