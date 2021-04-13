/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include <unordered_map>
#include <fmt/format.h>
#include "HttpException.h"

/* 定义错误码区间划分 */

#define HTTP_FILTER_ERROR_START 1

namespace hku {

/**
 * 公共错误码
 */
enum HttpErrorCode {
    MISS_CONTENT = HTTP_FILTER_ERROR_START,
    INVALID_JSON_REQUEST,  // 请求数据无法解析为JSON
    MISS_TOKEN,
    UNAUTHORIZED,
    MISS_PARAMETER,  // 缺失参数，参数不能为空 必填参数不能为空（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER,  // 参数值填写错误（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER_TYPE  // 参数类型错误（各个业务接口返回各个接口的参数）
};

#define HTTP_CHECK(expr, errcode, ...)                          \
    {                                                           \
        if (!(expr)) {                                          \
            throw HttpError(errcode, fmt::format(__VA_ARGS__)); \
        }                                                       \
    }

class HttpError : public HttpException {
public:
    HttpError() = delete;
    HttpError(int errcode, const char* msg)
    : HttpException(NNG_HTTP_STATUS_BAD_REQUEST, msg), m_errcode(errcode) {}

    HttpError(int errcode, const std::string& msg)
    : HttpException(NNG_HTTP_STATUS_BAD_REQUEST, msg), m_errcode(errcode) {}

    virtual std::string msg() const noexcept override {
        return fmt::format(R"({{"errcode": {}, "errmsg": "{}"}})", m_errcode, what());
    }

    int errcode() const noexcept {
        return m_errcode;
    }

private:
    int m_errcode;
};

}  // namespace hku