/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-05
 *     Author: fasiondog
 */

#pragma once

#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>

#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

#include <hikyuu/exception.h>

namespace hku {

#define CLASS_LOGGER(cls)                                                                     \
private:                                                                                      \
    inline static std::shared_ptr<spdlog::async_logger> ms_##cls_logger;                      \
                                                                                              \
public:                                                                                       \
    static std::shared_ptr<spdlog::async_logger> logger() {                                   \
        static std::once_flag oc;                                                             \
        std::call_once(oc, [&]() {                                                            \
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();       \
            stdout_sink->set_level(spdlog::level::trace);                                     \
                                                                                              \
            spdlog::init_thread_pool(8192, 1);                                                \
            std::vector<spdlog::sink_ptr> sinks{stdout_sink};                                 \
            ms_##cls_logger = std::make_shared<spdlog::async_logger>(                         \
              #cls, sinks.begin(), sinks.end(), spdlog::thread_pool(),                        \
              spdlog::async_overflow_policy::block);                                          \
                                                                                              \
            ms_##cls_logger->set_level(spdlog::level::trace);                                 \
            ms_##cls_logger->flush_on(spdlog::level::trace);                                  \
            ms_##cls_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^" #cls "-%L%$] - %v [%!]"); \
            spdlog::register_logger(ms_##cls_logger);                                         \
        });                                                                                   \
        return ms_##cls_logger;                                                               \
    }

#define CLS_TRACE(...) SPDLOG_LOGGER_TRACE(logger(), __VA_ARGS__)
#define CLS_DEBUG(...) SPDLOG_LOGGER_DEBUG(logger(), __VA_ARGS__)
#define CLS_INFO(...) SPDLOG_LOGGER_INFO(logger(), __VA_ARGS__)
#define CLS_WARN(...) SPDLOG_LOGGER_WARN(logger(), __VA_ARGS__)
#define CLS_ERROR(...) SPDLOG_LOGGER_ERROR(logger(), __VA_ARGS__)
#define CLS_FATAL(...) SPDLOG_LOGGER_CRITICAL(logger(), __VA_ARGS__)

/**
 * 满足指定条件时，打印 TRACE 信息
 * @param expr 指定条件
 */
#define CLS_TRACE_IF(expr, ...) \
    if (expr) {                 \
        CLS_TRACE(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 */
#define CLS_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        CLS_DEBUG(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 */
#define CLS_INFO_IF(expr, ...) \
    if (expr) {                \
        CLS_INFO(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 */
#define CLS_WARN_IF(expr, ...) \
    if (expr) {                \
        CLS_WARN(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 */
#define CLS_ERROR_IF(expr, ...) \
    if (expr) {                 \
        CLS_ERROR(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 */
#define CLS_FATAL_IF(expr, ...) \
    if (expr) {                 \
        CLS_FATAL(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * 满足指定条件时，打印 TRACE 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        CLS_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        CLS_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        CLS_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        CLS_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        CLS_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define CLS_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        CLS_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

///////////////////////////////////////////////////////////////////////////////
//
// clang/gcc 下使用 __PRETTY_FUNCTION__ 会包含函数参数，可以在编译时指定
// #define CLS_FUNCTION __PRETTY_FUNCTION__
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CLS_FUNCTION
#define CLS_FUNCTION __FUNCTION__
#endif

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define CLS_CHECK(expr, ...)                                                                   \
    do {                                                                                       \
        if (!(expr)) {                                                                         \
            throw hku::exception(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr,               \
                                             fmt::format(__VA_ARGS__), __FUNCTION__, __FILE__, \
                                             __LINE__));                                       \
        }                                                                                      \
    } while (0)

/**
 * 若表达式为 false，将抛出指定的异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr, fmt::format(__VA_ARGS__), \
                                     __FUNCTION__, __FILE__, __LINE__));                           \
        }                                                                                          \
    } while (0)

#if CLS_DISABLE_ASSERT
#define CLS_ASSERT(expr)
#define CLS_ASSERT_M(expr, ...)

#else /* #if CLS_DISABLE_ASSERT */

/**
 * 若表达式为 false，将抛出 hku::exception 异常
 * @note 仅用于内部入参检查，编译时可通过 CLS_DISABLE_ASSERT 宏关闭
 */
#define CLS_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("ASSERT({})", #expr));                        \
            CLS_ERROR(err_msg);                                                           \
            throw hku::exception(                                                         \
              fmt::format("{} [{}] ({}:{})", err_msg, __FUNCTION__, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 仅用于内部入参检查，编译时可通过 CLS_DISABLE_ASSERT 宏关闭
 */
#define CLS_ASSERT_M(expr, ...)                                                                 \
    do {                                                                                        \
        if (!(expr)) {                                                                          \
            std::string err_msg(fmt::format("ASSERT({}) {}", #expr, fmt::format(__VA_ARGS__))); \
            CLS_ERROR(err_msg);                                                                 \
            throw hku::exception(                                                               \
              fmt::format("{} [{}] ({}:{})", err_msg, __FUNCTION__, __FILE__, __LINE__));       \
        }                                                                                       \
    } while (0)

#endif /* #if CLS_DISABLE_ASSERT */

/** 抛出 hku::exception 及传入信息 */
#define CLS_THROW(...)                                                                           \
    do {                                                                                         \
        throw hku::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                         __FUNCTION__, __FILE__, __LINE__));                     \
    } while (0)

/** 抛出指定异常及传入信息 */
#define CLS_THROW_EXCEPTION(except, ...)                                                 \
    do {                                                                                 \
        throw except(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                 __FUNCTION__, __FILE__, __LINE__));                     \
    } while (0)

}  // namespace hku