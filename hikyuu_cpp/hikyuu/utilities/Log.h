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
#include "cppdef.h"

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

/** Log level */
enum LOG_LEVEL {
    LOG_TRACE = SPDLOG_LEVEL_TRACE,     ///< Trace
    LOG_DEBUG = SPDLOG_LEVEL_DEBUG,     ///< Debug
    LOG_INFO = SPDLOG_LEVEL_INFO,       ///< General information
    LOG_WARN = SPDLOG_LEVEL_WARN,       ///< Warning
    LOG_ERROR = SPDLOG_LEVEL_ERROR,     ///< Error
    LOG_FATAL = SPDLOG_LEVEL_CRITICAL,  ///< Fatal
    LOG_OFF = SPDLOG_LEVEL_OFF,         ///< Turn off the log printing
};

/**
 * Initialize the logger
 * @param not_use_color do not use the colored output
 * @param filename the log file name; it is "./hikyuu.log" in the current directory by default when
 *                 it is empty; you need to guarantee that the directory exists and is writable
 */
void HKU_UTILS_API initLogger(bool not_use_color = false,
                              const std::string& filename = std::string());

/**
 * Get the current log level
 * @return
 */
LOG_LEVEL HKU_UTILS_API get_log_level();

/**
 * Set the log level
 * @param level the given log level
 */
void HKU_UTILS_API set_log_level(LOG_LEVEL level);

std::shared_ptr<spdlog::logger> HKU_UTILS_API getHikyuuLogger();

#define HKU_TRACE(...) SPDLOG_LOGGER_TRACE(::hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_DEBUG(...) SPDLOG_LOGGER_DEBUG(::hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_INFO(...) SPDLOG_LOGGER_INFO(::hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_WARN(...) SPDLOG_LOGGER_WARN(::hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_ERROR(...) SPDLOG_LOGGER_ERROR(::hku::getHikyuuLogger(), __VA_ARGS__)
#define HKU_FATAL(...) SPDLOG_LOGGER_CRITICAL(::hku::getHikyuuLogger(), __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
//
// Under clang/gcc __PRETTY_FUNCTION__ contains the function parameters, it can be specified at
// compile time
// #define HKU_FUNCTION __PRETTY_FUNCTION__
//
///////////////////////////////////////////////////////////////////////////////
#ifndef HKU_FUNCTION
#define HKU_FUNCTION __FUNCTION__
#endif

#if !HKU_ENABLE_STACK_TRACE
#if CPP_STANDARD >= CPP_STANDARD_20
/**
 * If the expression is false an hku::exception is thrown with the passed information
 * @note Used for checking the external input parameters and the results
 */
#define HKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) [[unlikely]] {                                                              \
            throw ::hku::exception(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,           \
                                               fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, \
                                               __LINE__));                                       \
        }                                                                                        \
    } while (0)

/**
 * If the expression is false the given exception is thrown with the passed information
 * @note Used for checking the external input parameters and the results
 */
#define HKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) [[unlikely]] {                                                                \
            throw except(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#else
#define HKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            throw ::hku::exception(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,           \
                                               fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, \
                                               __LINE__));                                       \
        }                                                                                        \
    } while (0)

#define HKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20
#else
#define HKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            std::string errmsg = fmt::format(__VA_ARGS__);                                       \
            errmsg = fmt::format("{}\n {}", errmsg, to_string(boost::stacktrace::stacktrace())); \
            throw ::hku::exception(fmt::format("HKU_CHECK({}) {} [{}] ({}:{})", #expr, errmsg,   \
                                               HKU_FUNCTION, __FILE__, __LINE__));               \
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
 * If the expression is false an hku::exception is thrown
 * @note Used for checking the internal input parameters only; it can be disabled at compile time
 *       with the HKU_DISABLE_ASSERT macro
 */
#define HKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HKU_ASSERT({})\n{}", #expr,                  \
                                            to_string(boost::stacktrace::stacktrace()))); \
            throw ::hku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

#else
#if CPP_STANDARD >= CPP_STANDARD_20
#define HKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) [[unlikely]] {                                                       \
            std::string err_msg(fmt::format("HKU_ASSERT({})", #expr));                    \
            throw ::hku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)
#else
#define HKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HKU_ASSERT({})", #expr));                    \
            throw ::hku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20
#endif  // #if HKU_ENABLE_STACK_TRACE

#if !HKU_ENABLE_STACK_TRACE
/** Throw an hku::exception with the passed information */
#define HKU_THROW(...)                                                                             \
    do {                                                                                           \
        throw ::hku::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                           HKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

/** Throw the given exception with the passed information */
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
        throw ::hku::exception(                                                                 \
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
 * Print the TRACE information when the given condition is satisfied
 * @param expr the given condition
 */
#define HKU_TRACE_IF(expr, ...) \
    if (expr) {                 \
        HKU_TRACE(__VA_ARGS__); \
    }

/**
 * Print the DEBUG information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HKU_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        HKU_DEBUG(__VA_ARGS__); \
    }

/**
 * Print the INFO information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HKU_INFO_IF(expr, ...) \
    if (expr) {                \
        HKU_INFO(__VA_ARGS__); \
    }

/**
 * Print the WARN information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HKU_WARN_IF(expr, ...) \
    if (expr) {                \
        HKU_WARN(__VA_ARGS__); \
    }

/**
 * Print the ERROR information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HKU_ERROR_IF(expr, ...) \
    if (expr) {                 \
        HKU_ERROR(__VA_ARGS__); \
    }

/**
 * Print the FATAL information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HKU_FATAL_IF(expr, ...) \
    if (expr) {                 \
        HKU_FATAL(__VA_ARGS__); \
    }

/**
 * Return the given value when the given condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * Print the TRACE information and return the given value when the given condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the DEBUG information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the INFO information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HKU_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * Print the WARN information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HKU_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * Print the ERROR information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the FATAL information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HKU_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HKU_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

/** Used for the printing in catch (...), it reduces the size of the compiled code */
#define HKU_THROW_UNKNOWN HKU_THROW("Unknown error!")
#define HKU_TRACE_UNKNOWN HKU_TRACE("Unknown error!")
#define HKU_DEBUG_UNKNOWN HKU_DEBUG("Unknown error!")
#define HKU_INFO_UNKNOWN HKU_INFO("Unknown error!");
#define HKU_ERROR_UNKNOWN HKU_ERROR("Unknown error!");
#define HKU_FATAL_UNKNOWN HKU_FATAL("Unknown error!");

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

#if CPP_STANDARD >= CPP_STANDARD_20
#define CLS_CHECK(expr, ...)                                                                    \
    do {                                                                                        \
        if (!(expr)) [[unlikely]] {                                                             \
            throw ::hku::exception(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, \
                                               #expr, fmt::format(__VA_ARGS__), HKU_FUNCTION,   \
                                               __FILE__, __LINE__));                            \
        }                                                                                       \
    } while (0)

#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) [[unlikely]] {                                                                \
            throw except(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, #expr,       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#else
#define CLS_CHECK(expr, ...)                                                                    \
    do {                                                                                        \
        if (!(expr)) {                                                                          \
            throw ::hku::exception(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, \
                                               #expr, fmt::format(__VA_ARGS__), HKU_FUNCTION,   \
                                               __FILE__, __LINE__));                            \
        }                                                                                       \
    } while (0)

#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, #expr,       \
                                     fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20

#define CLS_THROW(...)                                                                       \
    do {                                                                                     \
        throw ::hku::exception(fmt::format("[{}] EXCEPTION: {} [{}] ({}:{})", ms_logger,     \
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
