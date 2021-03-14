/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <exception>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

namespace hku {

#if !defined(__clang__) && !defined(__GNUC__)
class HttpError : public std::exception {
public:
    HttpError() : std::exception("Unknown exception!") {}
    HttpError(int http_status) : HttpError(http_status, "") {}
    HttpError(int http_status, const std::string &msg)
    : std::exception(msg.c_str()), m_http_status(http_status) {}
    HttpError(int http_status, const char *msg) : std::exception(msg), m_http_status(http_status) {}
    virtual ~HttpError() noexcept {}

    int status() const noexcept {
        return m_http_status;
    }

    virtual std::string msg() const noexcept {
        return std::string(what());
    }

private:
    int m_http_status{NNG_HTTP_STATUS_BAD_REQUEST};
};

#else
// llvm 中的 std::exception 不接受参数
class HttpError : public std::exception {
public:
    HttpError() : m_msg("Unknown exception!") {}
    HttpError(int http_status) : HttpError(http_status, "") {}
    HttpError(int http_status, const char *msg) : m_msg(msg), m_http_status(http_status) {}
    HttpError(int http_status, const std::string &msg) : m_msg(msg), m_http_status(http_status) {}
    virtual ~HttpError() noexcept {}
    virtual const char *what() const noexcept {
        return m_msg.c_str();
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

}  // namespace hku