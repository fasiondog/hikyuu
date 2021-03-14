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
#include "RestError.h"

namespace hku {

class RestFilterError : public HttpError {
public:
    enum class Code {
        MISS_TOKEN = REST_FILTER_ERROR_START,
        UNAUTHORIZED,
        INVALID_CONTENT_TYPE,
    };

public:
    RestFilterError() = delete;
    RestFilterError(Code errcode) : HttpError(NNG_HTTP_STATUS_BAD_REQUEST), m_errcode(errcode) {}

    virtual std::string msg() const noexcept override {
        auto iter = ms_err_msg.find(m_errcode);
        return iter != ms_err_msg.end()
                 ? fmt::format(R"({{"errcode":{},"error":"{}"}})", m_errcode, iter->second)
                 : fmt::format(R"("errcode":{}, "error": "No error description information."}})",
                               m_errcode);
    }

private:
    Code m_errcode;

private:
    static std::unordered_map<Code, const char *> ms_err_msg;
};

}  // namespace hku