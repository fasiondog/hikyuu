/*
 * exception.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-23
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_EXCEPTION_H
#define HIKYUU_UTILITIES_EXCEPTION_H

#include <string>
#include <exception>
#include <fmt/format.h>

namespace hku {

/**
 * @ingroup Utilities
 * @addtogroup Exception Exception 异常处理
 * @{
 */

#if !defined(__clang__) && !defined(__GNUC__)
class exception : public std::exception {
public:
    exception() : std::exception("Unknow exception!") {}
    exception(const std::string& msg) : std::exception(msg.c_str()) {}
    exception(const char* msg) : std::exception(msg) {}
};

#else
// llvm 中的 std::exception 不接受参数
class exception : public std::exception {
public:
    exception() : m_msg("Unknow exception!") {}
    exception(const char *msg) : m_msg(msg) {}
    exception(const std::string &msg) : m_msg(msg) {}
    virtual ~exception() noexcept {}
    virtual const char *what() const noexcept {
        return m_msg.c_str();
    }

protected:
    std::string m_msg;
};
#endif /* #ifdef __clang__ */

/** 若表达式为 false，将抛出 hku::exception 异常 */
#define HKU_ASSERT(expr)                                                                      \
    {                                                                                         \
        if (!(expr))                                                                          \
            throw hku::exception(                                                             \
              fmt::format("HKU_ASSERT: {} ({}:{} {})", #expr, __FILE__, __LINE__, __func__)); \
    }

/** 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息 */
#define HKU_ASSERT_M(expr, ...)                                                               \
    {                                                                                         \
        if (!(expr)) {                                                                        \
            string s = fmt::format(__VA_ARGS__);                                              \
            throw hku::exception(fmt::format("HKU_ASSERT: {} [MSG: {}] ({}:{} {})", #expr, s, \
                                             __FILE__, __LINE__, __func__));                  \
        }                                                                                     \
    }

/** 抛出 hku::exception 及传入信息 */
#define HKU_THROW(...)                                                                    \
    {                                                                                     \
        string s = fmt::format(__VA_ARGS__);                                              \
        s = fmt::format("HKU_EXCEPTION: {} ({}:{} {})", s, __FILE__, __LINE__, __func__); \
        throw hku::exception(s);                                                          \
    }

/** 抛出指定异常及传入信息 */
#define HKU_THROW_EXCEPTION(except, ...)                                                  \
    {                                                                                     \
        string s = fmt::format(__VA_ARGS__);                                              \
        s = fmt::format("HKU_EXCEPTION: {} ({}:{} {})", s, __FILE__, __LINE__, __func__); \
        throw except(s);                                                                  \
    }

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_EXCEPTION_H */
