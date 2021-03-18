/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-12
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <exception>
#include <fmt/format.h>

namespace yyjson {

#if !defined(__clang__) && !defined(__GNUC__)
class exception : public std::exception {
public:
    exception() : std::exception("Unknown exception!") {}
    exception(const std::string& msg)  // cppcheck-suppress noExplicitConstructor
    : std::exception(msg.c_str()) {}
    exception(const char* msg) : std::exception(msg) {}  // cppcheck-suppress noExplicitConstructor
};

#else
// llvm 中的 std::exception 不接受参数
class exception : public std::exception {
public:
    exception() : m_msg("Unknown exception!") {}
    exception(const char *msg) : m_msg(msg) {}         // cppcheck-suppress noExplicitConstructor
    exception(const std::string &msg) : m_msg(msg) {}  // cppcheck-suppress noExplicitConstructor
    virtual ~exception() noexcept {}
    virtual const char *what() const noexcept {
        return m_msg.c_str();
    }

protected:
    std::string m_msg;
};
#endif /* #ifdef __clang__ */

class bad_cast : public exception {
public:
    bad_cast() : exception("Type conversion failed!") {}
    bad_cast(const char* msg) : exception(msg) {}
    bad_cast(const std::string& msg) : exception(msg) {}
    virtual ~bad_cast() {}
};

#define YYJSON_CHECK(expr, ...)                                                                   \
    do {                                                                                          \
        if (!(expr)) {                                                                            \
            throw yyjson::exception(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr,               \
                                                fmt::format(__VA_ARGS__), __FUNCTION__, __FILE__, \
                                                __LINE__));                                       \
        }                                                                                         \
    } while (0)

#define YYJSON_CHECK_THROW(expr, except, ...)                                                      \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr, fmt::format(__VA_ARGS__), \
                                     __FUNCTION__, __FILE__, __LINE__));                           \
        }                                                                                          \
    } while (0)

#define YYJSON_THROW(...)                                                                     \
    do {                                                                                      \
        throw yyjson::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})",                     \
                                            fmt::format(__VA_ARGS__), __FUNCTION__, __FILE__, \
                                            __LINE__));                                       \
    } while (0)

#define YYJSON_THROW_EXCEPTION(except, ...)                                              \
    do {                                                                                 \
        throw except(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                 __FUNCTION__, __FILE__, __LINE__));                     \
    } while (0)

}  // namespace yyjson