/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include <unordered_map>
#include <fmt/format.h>
#include "../http/HttpError.h"

namespace hku {

class RestError : public HttpError {
public:
    enum error {
        INVALID_CONTENT = 1,
        MISS_TOKEN = 2,
        UNAUTHORIZED = 3,
    };

public:
    RestError() : HttpError(NNG_HTTP_STATUS_BAD_REQUEST) {}

    virtual std::string msg() const noexcept override {
        return fmt::format(R"({{"errcode": "{}", "msg": "{}"}})", 0, ms_err_msg[0]);
    }

private:
    static std::unordered_map<long, const char *> ms_err_msg;
};

}  // namespace hku