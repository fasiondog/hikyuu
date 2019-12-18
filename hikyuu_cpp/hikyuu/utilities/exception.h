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
#include "../Log.h"

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

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define HKU_CHECK(expr, ...)                                                             \
    {                                                                                    \
        if (!(expr)) {                                                                   \
            std::string s(fmt::format("CHECK({}) {}", #expr, fmt::format(__VA_ARGS__))); \
            HKU_ERROR(s);                                                                \
            throw hku::exception(s);                                                     \
        }                                                                                \
    }

#if HKU_DISABLE_ASSERT
#define HKU_ASSERT(expr)
#define HKU_ASSERT_M(expr, ...)

#else /* #if HKU_DISABLE_ASSERT */

/**
 * 若表达式为 false，将抛出 hku::exception 异常
 * @note 仅用于内部入参检查，编译时可通过 HKU_DISABLE_ASSERT 宏关闭
 */
#define HKU_ASSERT(expr)                                     \
    {                                                        \
        if (!(expr)) {                                       \
            std::string s(fmt::format("ASSERT({})", #expr)); \
            HKU_ERROR(s);                                    \
            throw hku::exception(s);                         \
        }                                                    \
    }

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 仅用于内部入参检查，编译时可通过 HKU_DISABLE_ASSERT 宏关闭
 */
#define HKU_ASSERT_M(expr, ...)                                                           \
    {                                                                                     \
        if (!(expr)) {                                                                    \
            std::string s(fmt::format("ASSERT({}) {}", #expr, fmt::format(__VA_ARGS__))); \
            HKU_ERROR(s);                                                                 \
            throw hku::exception(s);                                                      \
        }                                                                                 \
    }

#endif /* #if HKU_DISABLE_ASSERT */

/** 抛出 hku::exception 及传入信息 */
#define HKU_THROW(...)                                                         \
    {                                                                          \
        std::string s(fmt::format("EXCEPTION: {}", fmt::format(__VA_ARGS__))); \
        HKU_ERROR(s);                                                          \
        throw hku::exception(s);                                               \
    }

/** 抛出指定异常及传入信息 */
#define HKU_THROW_EXCEPTION(except, ...)                                       \
    {                                                                          \
        std::string s(fmt::format("EXCEPTION: {}", fmt::format(__VA_ARGS__))); \
        HKU_ERROR(s);                                                          \
        throw except(s);                                                       \
    }

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_EXCEPTION_H */
