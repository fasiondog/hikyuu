/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <exception>
#include <fmt/format.h>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

namespace hku {

#if !defined(__clang__) && !defined(__GNUC__)
class HttpError : public std::exception {
public:
    HttpError() : std::exception("Unknown http error!"), m_name("HttpError") {}
    explicit HttpError(const char* name) : std::exception("Unknown http error!"), m_name(name) {}

    HttpError(const char* name, int http_status)
    : HttpError(name, http_status, fmt::format("Http status {}", http_status)) {}

    HttpError(const char* name, int http_status, const std::string& msg)
    : std::exception(msg.c_str()),
      m_name(name),
      m_http_status(http_status),
      m_errcode(http_status) {}

    HttpError(const char* name, int http_status, const char* msg)
    : std::exception(msg), m_name(name), m_http_status(http_status), m_errcode(http_status) {}

    HttpError(const char* name, int http_status, int errcode, const std::string& msg)
    : std::exception(msg.c_str()), m_name(name), m_http_status(http_status), m_errcode(errcode) {}

    HttpError(const char* name, int http_status, int errcode, const char* msg)
    : std::exception(msg), m_name(name), m_http_status(http_status), m_errcode(errcode) {}

    virtual ~HttpError() noexcept {}

    const std::string& name() const noexcept {
        return m_name;
    }

    int status() const noexcept {
        return m_http_status;
    }

    int errcode() const noexcept {
        return m_errcode;
    }

protected:
    std::string m_name;
    int m_http_status{NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR};
    int m_errcode{NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR};
};

#else
// llvm 中的 std::exception 不接受参数
class HttpError : public std::exception {
public:
    HttpError() : m_name("HttpErrot"), m_msg("Unknown http error!"){};
    explicit HttpError(const char* name) : m_name(name), m_msg("Unknown http error!") {}
    HttpError(const char* name, int http_status)
    : HttpError(name, http_status, fmt::format("Http status {}", http_status)) {}

    HttpError(const char* name, int http_status, const char* msg)
    : m_name(name), m_msg(msg), m_http_status(http_status), m_errcode(http_status) {}
    HttpError(const char* name, int http_status, const std::string& msg)
    : m_name(name), m_msg(msg), m_http_status(http_status), m_errcode(http_status) {}

    HttpError(const char* name, int http_status, int errcode, const char* msg)
    : m_name(name), m_msg(msg), m_http_status(http_status), m_errcode(errcode) {}
    HttpError(const char* name, int http_status, int errcode, const std::string& msg)
    : m_name(name), m_msg(msg), m_http_status(http_status), m_errcode(errcode) {}

    virtual ~HttpError() noexcept {}
    virtual const char* what() const noexcept {
        return m_msg.c_str();
    }

    const std::string& name() const noexcept {
        return m_name;
    }

    int status() const noexcept {
        return m_http_status;
    }

    int errcode() const noexcept {
        return m_errcode;
    }

protected:
    std::string m_name;
    std::string m_msg;
    int m_http_status{NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR};
    int m_errcode{NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR};
};
#endif /* #ifdef __clang__ */

/**
 * Http 400: 请求参数错误
 */
class HttpBadRequestError : public HttpError {
public:
    HttpBadRequestError() : HttpError("HttpBadRequestError") {}
    HttpBadRequestError(int errcode, const char* msg)
    : HttpError("HttpBadRequestError", NNG_HTTP_STATUS_BAD_REQUEST, errcode, msg) {}
    HttpBadRequestError(int errcode, const std::string& msg)
    : HttpError("HttpBadRequestError", NNG_HTTP_STATUS_BAD_REQUEST, errcode, msg) {}
};

/**
 * Http 401: 当前请求需要用户验证
 */
class HttpUnauthorizedError : public HttpError {
public:
    HttpUnauthorizedError() : HttpError("HttpUnauthorizedError") {}
    HttpUnauthorizedError(int errcode, const char* msg)
    : HttpError("HttpUnauthorizedError", NNG_HTTP_STATUS_UNAUTHORIZED, errcode, msg) {}
    HttpUnauthorizedError(int errcode, const std::string& msg)
    : HttpError("HttpUnauthorizedError", NNG_HTTP_STATUS_UNAUTHORIZED, errcode, msg) {}
};

class HttpNotAcceptableError : public HttpError {
public:
    enum NotAcceptableErrorCode {
        UNSUPPORT_CONTENT_ENCODING = 4060001,  // 不支持的内容编码
    };

    HttpNotAcceptableError() : HttpError("HttpNotAcceptableError") {}
    HttpNotAcceptableError(int errcode, const char* msg)
    : HttpError("HttpNotAcceptableError", NNG_HTTP_STATUS_NOT_ACCEPTABLE, errcode, msg) {}
    HttpNotAcceptableError(int errcode, const std::string& msg)
    : HttpError("HttpNotAcceptableError", NNG_HTTP_STATUS_NOT_ACCEPTABLE, errcode, msg) {}
};

/**
 * 公共错误码
 */
enum AuthorizeErrorCode {
    MISS_TOKEN = 10000,  // 缺失令牌
    FAILED_AUTHORIZED,   // 鉴权失败
    AUTHORIZE_EXPIRED,   // 鉴权过期
};

enum BadRequestErrorCode {
    INVALID_JSON_REQUEST = 20001,  // 请求数据无法解析为JSON
    MISS_PARAMETER,  // 缺失参数，参数不能为空 必填参数不能为空（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER,  // 参数值填写错误（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER_TYPE  // 参数类型错误（各个业务接口返回各个接口的参数）
};

#define AUTHORIZE_CHECK(expr, errcode, ...)                                 \
    {                                                                       \
        if (!(expr)) {                                                      \
            throw HttpUnauthorizedError(errcode, fmt::format(__VA_ARGS__)); \
        }                                                                   \
    }

#define REQ_CHECK(expr, errcode, ...)                                     \
    {                                                                     \
        if (!(expr)) {                                                    \
            throw HttpBadRequestError(errcode, fmt::format(__VA_ARGS__)); \
        }                                                                 \
    }

}  // namespace hku