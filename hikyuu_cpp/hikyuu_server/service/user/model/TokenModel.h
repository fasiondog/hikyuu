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
    TABLE_BIND2(TokenModel, admin_token, userid, token)

public:
    uint64_t getUserId() const {
        return userid;
    }

    void setUserId(uint64_t id) {
        userid = id;
    }

    string getToken() const {
        return token;
    }

    void setToken(const string& token) {
        this->token = token;
    }

private:
    uint64_t userid;
    string token;
};

}  // namespace hku