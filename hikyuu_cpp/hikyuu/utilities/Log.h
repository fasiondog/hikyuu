/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKUU_LOG_H_
#define HIKUU_LOG_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "config.h"
#include "exception.h"

#ifndef HKU_LOG_ACTIVE_LEVEL
#define HKU_LOG_ACTIVE_LEVEL 0
#endif

// clang-format off
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL HKU_LOG_ACTIVE_LEVEL
#endif

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#if HKU_USE_SPDLOG_ASYNC_LOGGER
    #include "spdlog/async.h"
#endif
// clang-format on

#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

#ifndef HKU_ENABLE_STACK_TRACE
#define HKU_ENABLE_STACK_TRACE 0
#endif

#if HKU_ENABLE_STACK_TRACE
#include <boost/stacktrace.hpp>
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**********************************************
 * Use SPDLOG for logging
 *********************************************/

/** 日志级别 */
enum LOG_LEVEL {
    LOG_TRACE = SPDLOG_LEVEL_TRACE,     ///< 跟踪
    LOG_DEBUG = SPDLOG_LEVEL_DEBUG,     ///< 调试
    LOG_INFO = SPDLOG_LEVEL_INFO,       ///< 一般信息
    LOG_WARN = SPDLOG_LEVEL_WARN,       ///< 告警
    LOG_ERROR = SPDLOG_LEVEL_ERROR,     ///< 错误
    LOG_FATAL = SPDLOG_LEVEL_CRITICAL,  ///< 致命
    LOG_OFF = SPDLOG_LEVEL_OFF,         ///< 关闭日志打印
};

/**
 * 初始化 logger
 * @param not_use_color 不使用彩色输出
 * @param filename 日志文件名，为空时默认为当前目录下 "./hikyuu.log"，需自行保存存放目录存在且可写入
 */
void HKU_UTILS_API initLogger(bool not_use_color = false,
                              const std::string& filename = std::string());

/**
 * 获取当前日志级别
 * @return
 */
LOG_LEVEL HKU_UTILS_API get_log_level();

/**
 * 设置日志级别
 * @param level 指定的日志级别
 */
void HKU_UTILS_API set_log_level(LOG_LEVEL level);

std::shared_ptr<spdlog::logger> HKU_UTILS_API getHikyuuLogger();

#define HKU_TRACE(...) SPDLOG_LOGGER_TRACE(hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_DEBUG(...) SPDLOG_LOGGER_DEBUG(hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_INFO(...) SPDLOG_LOGGER_INFO(hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_WARN(...) SPDLOG_LOGGER_WARN(hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_ERROR(...) SPDLOG_LOGGER_ERROR(hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_FATAL(...) SPDLOG_LOGGER_CRITICAL(hku::getHikyuuLogger(), __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
//
// clang/gcc 下使用 __PRETTY_FUNCTION__ 会包含函数参数，可以在编译时指定
// #define HKU_FUNCTION __PRETTY_FUNCTION__
//
///////////////////////////////////////////////////////////////////////////////
#ifndef HKU_FUNCTION
#define HKU_FUNCTION __FUNCTION__
#endif

#if !HKU_ENABLE_STACK_TRACE
/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define HKU_CHECK(expr, ...)                                                                   \
    do {                                                                                       \
        if (!(expr)) {                                                                         \
            throw hku::exception(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,           \
                                             fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, \
                                             __LINE__));                                       \
        }                                                                                      \
    } while (0)

/**
 * 若表达式为 false，将抛出指定的异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define HKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)

#else
#define HKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            std::string errmsg = fmt::format(__VA_ARGS__);                                       \
            errmsg = fmt::format("{}\n {}", errmsg, to_string(boost::stacktrace::stacktrace())); \
            throw hku::exception(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr, errmsg,     \
                                             HKU_FUNCTION, __FILE__, __LINE__));                 \
        }                                                                                        \
    } while (0)

#define HKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            std::string errmsg = fmt::format(__VA_ARGS__);                                         \
            errmsg = fmt::format("{}\n {}", errmsg, to_string(boost::stacktrace::stacktrace()));   \
            throw except(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr, errmsg, HKU_FUNCTION, \
                                     __FILE__, __LINE__));                                         \
        }                                                                                          \
    } while (0)
#endif  // #if !HKU_ENABLE_STACK_TRACE

#if HKU_ENABLE_STACK_TRACE
/**
 * 若表达式为 false，将抛出 hku::exception 异常
 * @note 仅用于内部入参检查，编译时可通过 HKU_DISABLE_ASSERT 宏关闭
 */
#define HKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HKU_ASSERT({})\n{}", #expr,                  \
                                            to_string(boost::stacktrace::stacktrace()))); \
            throw hku::exception(                                                         \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

#else
#define HKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HKU_ASSERT({})", #expr));                    \
            throw hku::exception(                                                         \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

#endif  // #if HKU_ENABLE_STACK_TRACE

#if !HKU_ENABLE_STACK_TRACE
/** 抛出 hku::exception 及传入信息 */
#define HKU_THROW(...)                                                                           \
    do {                                                                                         \
        throw hku::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                         HKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

/** 抛出指定异常及传入信息 */
#define HKU_THROW_EXCEPTION(except, ...)                                                 \
    do {                                                                                 \
        throw except(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                 HKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

#else
#define HKU_THROW(...)                                                                          \
    do {                                                                                        \
        std::string errmsg(fmt::format("{}\n {}", fmt::format(__VA_ARGS__),                     \
                                       to_string(boost::stacktrace::stacktrace())));            \
        throw hku::exception(                                                                   \
          fmt::format("EXCEPTION: {} [{}] ({}:{})", errmsg, HKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)

#define HKU_THROW_EXCEPTION(except, ...)                                                        \
    do {                                                                                        \
        std::string errmsg(fmt::format("{}\n {}", fmt::format(__VA_ARGS__),                     \
                                       to_string(boost::stacktrace::stacktrace())));            \
        throw except(                                                                           \
          fmt::format("EXCEPTION: {} [{}] ({}:{})", errmsg, HKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)
#endif  // #if !HKU_ENABLE_STACK_TRACE

/**
 * 满足指定条件时，打印 TRACE 信息
 * @param expr 指定条件
 */
#define HKU_TRACE_IF(expr, ...) \
    if (expr) {                 \
        HKU_TRACE(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 */
#define HKU_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        HKU_DEBUG(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 */
#define HKU_INFO_IF(expr, ...) \
    if (expr) {                \
        HKU_INFO(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 */
#define HKU_WARN_IF(expr, ...) \
    if (expr) {                \
        HKU_WARN(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 */
#define HKU_ERROR_IF(expr, ...) \
    if (expr) {                 \
        HKU_ERROR(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 */
#define HKU_FATAL_IF(expr, ...) \
    if (expr) {                 \
        HKU_FATAL(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * 满足指定条件时，打印 TRACE 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HKU_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HKU_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define HKU_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

/** 用于 catch (...) 中打印，减少编译后代码大小 */
extern std::string g_unknown_error_msg;
#define HKU_THROW_UNKNOWN HKU_THROW("{}", g_unknown_error_msg);
#define HKU_TRACE_UNKNOWN HKU_TRACE("{}", g_unknown_error_msg)
#define HKU_DEBUG_UNKNOWN HKU_DEBUG("{}", g_unknown_error_msg)
#define HKU_INFO_UNKNOWN HKU_INFO("{}", g_unknown_error_msg)
#define HKU_ERROR_UNKNOWN HKU_ERROR("{}", g_unknown_error_msg)
#define HKU_FATAL_UNKNOWN HKU_FATAL("{}", g_unknown_error_msg)

#if CPP_STANDARD >= CPP_STANDARD_20
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    static constexpr const char* ms_logger = #cls;
#elif CPP_STANDARD >= CPP_STANDARD_17
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    inline static const char* ms_logger = #cls;
#else
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    const char* ms_logger = #cls;
#endif

#define CLS_TRACE(...) HKU_TRACE(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG(...) HKU_DEBUG(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO(...) HKU_INFO(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN(...) HKU_WARN(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR(...) HKU_ERROR(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL(...) HKU_FATAL(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_TRACE_IF(expr, ...) \
    HKU_TRACE_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG_IF(expr, ...) \
    HKU_DEBUG_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO_IF(expr, ...) \
    HKU_INFO_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN_IF(expr, ...) \
    HKU_WARN_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR_IF(expr, ...) \
    HKU_ERROR_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL_IF(expr, ...) \
    HKU_FATAL_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_IF_RETURN(expr, ret) HKU_IF_RETURN(expr, ret)
#define CLS_TRACE_IF_RETURN(expr, ret, ...) \
    HKU_TRACE_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG_IF_RETURN(expr, ret, ...) \
    HKU_DEBUG_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO_IF_RETURN(expr, ret, ...) \
    HKU_INFO_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN_IF_RETURN(expr, ret, ...) \
    HKU_WARN_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR_IF_RETURN(expr, ret, ...) \
    HKU_ERROR_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL_IF_RETURN(expr, ret, ...) \
    HKU_FATAL_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_ASSERT HKU_ASSERT

#define CLS_CHECK(expr, ...)                                                                  \
    do {                                                                                      \
        if (!(expr)) {                                                                        \
            throw hku::exception(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, \
                                             #expr, fmt::format(__VA_ARGS__), HKU_FUNCTION,   \
                                             __FILE__, __LINE__));                            \
        }                                                                                     \
    } while (0)

#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, #expr,       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)

#define CLS_THROW(...)                                                                     \
    do {                                                                                   \
        throw hku::exception(fmt::format("[{}] EXCEPTION: {} [{}] ({}:{})", ms_logger,     \
                                         fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, \
                                         __LINE__));                                       \
    } while (0)

#define CLS_THROW_EXCEPTION(except, ...)                                                       \
    do {                                                                                       \
        throw except(fmt::format("[{}] EXCEPTION: {} [{}] ({}:{})", ms_logger,                 \
                                 fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)

/** @} */

} /* namespace hku */

#endif /* HIKUU_LOG_H_ */
