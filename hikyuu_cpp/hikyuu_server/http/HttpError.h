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
    HttpError() : std::exception("Unknown http error!") {}
    HttpError(int http_status) : HttpError(http_status, "") {}
    HttpError(int http_status, const std::string& msg)
    : std::exception(msg.c_str()), m_http_status(http_status) {}
    HttpError(int http_status, const char* msg) : std::exception(msg), m_http_status(http_status) {}
    virtual ~HttpError() noexcept {}

    static const char* name() noexcept {
        static const char* str_name = "HttpError";
        return str_name;
    }

    int status() const noexcept {
        return m_http_status;
    }

    virtual std::string msg() const noexcept {
        return std::string(what());
    }

protected:
    int m_http_status{NNG_HTTP_STATUS_BAD_REQUEST};
};

#else
// llvm 中的 std::exception 不接受参数
class HttpError : public std::exception {
public:
    HttpError() : m_msg("Unknown http error!") {}
    HttpError(int http_status) : HttpError(http_status, "") {}
    HttpError(int http_status, const char* msg) : m_msg(msg), m_http_status(http_status) {}
    HttpError(int http_status, const std::string& msg) : m_msg(msg), m_http_status(http_status) {}
    virtual ~HttpError() noexcept {}
    virtual const char* what() const noexcept {
        return m_msg.c_str();
    }

    static const char* name() noexcept {
        static const char* str_name = "HttpError";
        return str_name;
    }

    int status() const noexcept {
        return m_http_status;
    }

    virtual std::string msg() const noexcept {
        return m_msg;
    }

protected:
    std::string m_msg;
    int m_http_status{NNG_HTTP_STATUS_BAD_REQUEST};
};
#endif /* #ifdef __clang__ */

#define HANDLE_THROW(http_status, ...) \
    { throw HttpError(http_status, fmt::format(__VA_ARGS__)); }

#define HANDLE_CHECK(expr, http_status, ...)                        \
    {                                                               \
        if (!expr) {                                                \
            throw HttpError(http_status, fmt::format(__VA_ARGS__)); \
        }                                                           \
    }

#define HANDLE_CHECK_THROW(expr, error) \
    {                                   \
        if (!expr) {                    \
            throw error;                \
        }                               \
    }

class HttpBadRequestError : public HttpError {
public:
    HttpBadRequestError() = delete;
    HttpBadRequestError(int errcode, const char* msg)
    : HttpError(NNG_HTTP_STATUS_BAD_REQUEST, msg), m_errcode(errcode) {}

    HttpBadRequestError(int errcode, const std::string& msg)
    : HttpError(NNG_HTTP_STATUS_BAD_REQUEST, msg), m_errcode(errcode) {}

    static const char* name() noexcept {
        static const char* str_name = "HttpBadRequestError";
        return str_name;
    }

    virtual std::string msg() const noexcept override {
        return fmt::format(R"({{"result": false,"errcode":{}, "errmsg":"{}"}})", m_errcode, what());
    }

    int errcode() const noexcept {
        return m_errcode;
    }

private:
    int m_errcode;
};

class HttpUnauthorizedError : public HttpError {
public:
    HttpUnauthorizedError() = delete;
    HttpUnauthorizedError(int errcode, const char* msg)
    : HttpError(NNG_HTTP_STATUS_UNAUTHORIZED, msg), m_errcode(errcode) {}

    HttpUnauthorizedError(int errcode, const std::string& msg)
    : HttpError(NNG_HTTP_STATUS_UNAUTHORIZED, msg), m_errcode(errcode) {}

    static const char* name() noexcept {
        static const char* str_name = "HttpUnauthorizedError";
        return str_name;
    }

    virtual std::string msg() const noexcept override {
        return fmt::format(R"({{"result": false,"errcode":{}, "errmsg":"{}"}})", m_errcode, what());
    }

    int errcode() const noexcept {
        return m_errcode;
    }

private:
    int m_errcode;
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