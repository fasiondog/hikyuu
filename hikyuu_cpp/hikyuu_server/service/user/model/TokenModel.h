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
    TABLE_BIND1(admin_token, token)

public:
    TokenModel() = default;
    TokenModel(const std::string& tk) : token(tk) {}

    string getToken() const {
        return token;
    }

    void setToken(const string& token) {
        this->token = token;
    }

private:
    string token;
};

}  // namespace hku