/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-05
 *     Author: fasiondog
 */

#pragma once

#include <iostream>
#include <hikyuu/config.h>
#include <hikyuu/exception.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>

#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

#ifndef LOG_ACTIVE_LEVEL
#define LOG_ACTIVE_LEVEL 0
#endif

namespace hku {

#if LOG_ACTIVE_LEVEL <= 0
#define DEFAULT_LOGGER_LEVEL spdlog::level::trace
#elif LOG_ACTIVE_LEVEL <= 1
#define DEFAULT_LOGGER_LEVEL spdlog::level::debug
#elif LOG_ACTIVE_LEVEL <= 2
#define DEFAULT_LOGGER_LEVEL spdlog::level::info
#elif LOG_ACTIVE_LEVEL <= 2
#define DEFAULT_LOGGER_LEVEL spdlog::level::warn
#elif LOG_ACTIVE_LEVEL <= 3
#define DEFAULT_LOGGER_LEVEL spdlog::level::err
#elif LOG_ACTIVE_LEVEL <= 4
#define DEFAULT_LOGGER_LEVEL spdlog::level::critical
#endif

inline void init_server_logger() {
    static std::once_flag oc;
    std::call_once(oc, [&]() {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(DEFAULT_LOGGER_LEVEL);

        /*spdlog::init_thread_pool(8192, 1);
        std::vector<spdlog::sink_ptr> sinks{stdout_sink};
        auto logger = std::make_shared<spdlog::async_logger>("SERVER", sinks.begin(), sinks.end(),
                                                             spdlog::thread_pool(),
                                                             spdlog::async_overflow_policy::block);*/
        auto logger = std::make_shared<spdlog::logger>("SERVER", stdout_sink);
        logger->set_level(DEFAULT_LOGGER_LEVEL);
        logger->flush_on(DEFAULT_LOGGER_LEVEL);
        // logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^SERVER-%L%$] - %v (%s:%#)");
        logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [SERVER-%L] - %v (%s:%#)%$");
        spdlog::set_default_logger(logger);
    });
}

inline void set_logger_level(int level) {
    SPDLOG_INFO("set default logger level: {}", level);
    spdlog::set_level((spdlog::level::level_enum)level);
    spdlog::flush_on((spdlog::level::level_enum)level);
}

inline bool have_logger(const std::string& name) {
    return spdlog::get(name) ? true : false;
}

inline void set_logger_level(const std::string& name, int level) {
    auto logger = spdlog::get(name);
    if (logger) {
        SPDLOG_INFO("set {} logger level: {}", name, level);
        spdlog::get(name)->set_level((spdlog::level::level_enum)level);
        spdlog::get(name)->flush_on((spdlog::level::level_enum)level);
    } else {
        SPDLOG_INFO("Not exist {} logger", name);
    }
}

#if LOG_ACTIVE_LEVEL <= 0
#define APP_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define APP_TRACE(...)
#endif

#if LOG_ACTIVE_LEVEL <= 1
#define APP_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#else
#define APP_DEBUG(...)
#endif

#if LOG_ACTIVE_LEVEL <= 2
#define APP_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#else
#define APP_INFO(...)
#endif

#if LOG_ACTIVE_LEVEL <= 3
#define APP_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#else
#define APP_WARN(...)
#endif

#if LOG_ACTIVE_LEVEL <= 4
#define APP_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#else
#define APP_ERROR(...)
#endif

#if LOG_ACTIVE_LEVEL <= 5
#define APP_FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)
#else
#define APP_FATAL(...)
#endif

/**
 * 满足指定条件时，打印 TRACE 信息
 * @param expr 指定条件
 */
#define APP_TRACE_IF(expr, ...) \
    if (expr) {                 \
        APP_TRACE(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 */
#define APP_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        APP_DEBUG(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 */
#define APP_INFO_IF(expr, ...) \
    if (expr) {                \
        APP_INFO(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 */
#define APP_WARN_IF(expr, ...) \
    if (expr) {                \
        APP_WARN(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 */
#define APP_ERROR_IF(expr, ...) \
    if (expr) {                 \
        APP_ERROR(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 */
#define APP_FATAL_IF(expr, ...) \
    if (expr) {                 \
        APP_FATAL(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * 满足指定条件时，打印 TRACE 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        APP_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        APP_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        APP_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        APP_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        APP_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define APP_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        APP_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

///////////////////////////////////////////////////////////////////////////////
//
// clang/gcc 下使用 __PRETTY_FUNCTION__ 会包含函数参数，可以在编译时指定
// #define HKU_FUNCTION __PRETTY_FUNCTION__
//
///////////////////////////////////////////////////////////////////////////////
#ifndef HKU_FUNCTION
#define HKU_FUNCTION __FUNCTION__
#endif

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define APP_CHECK(expr, ...)                                                                   \
    do {                                                                                       \
        if (!(expr)) {                                                                         \
            throw hku::exception(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr,               \
                                             fmt::format(__VA_ARGS__), HKU_FUNCTION, __FILE__, \
                                             __LINE__));                                       \
        }                                                                                      \
    } while (0)

/**
 * 若表达式为 false，将抛出指定的异常, 并附带传入信息
 * @note 用于外部入参及结果检查
 */
#define APP_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("CHECK({}) {} [{}] ({}:{})", #expr, fmt::format(__VA_ARGS__), \
                                     HKU_FUNCTION, __FILE__, __LINE__));                           \
        }                                                                                          \
    } while (0)

#if HKU_DISABLE_ASSERT
#define LOG_ASSERT(expr)
#define LOG_ASSERT_M(expr, ...)

#else /* #if HKU_DISABLE_ASSERT */

/**
 * 若表达式为 false，将抛出 hku::exception 异常
 * @note 仅用于内部入参检查，编译时可通过 HKU_DISABLE_ASSERT 宏关闭
 */
#define LOG_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("ASSERT({})", #expr));                        \
            HKU_ERROR(err_msg);                                                           \
            throw hku::exception(                                                         \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

/**
 * 若表达式为 false，将抛出 hku::exception 异常, 并附带传入信息
 * @note 仅用于内部入参检查，编译时可通过 HKU_DISABLE_ASSERT 宏关闭
 */
#define LOG_ASSERT_M(expr, ...)                                                                 \
    do {                                                                                        \
        if (!(expr)) {                                                                          \
            std::string err_msg(fmt::format("ASSERT({}) {}", #expr, fmt::format(__VA_ARGS__))); \
            HKU_ERROR(err_msg);                                                                 \
            throw hku::exception(                                                               \
              fmt::format("{} [{}] ({}:{})", err_msg, HKU_FUNCTION, __FILE__, __LINE__));       \
        }                                                                                       \
    } while (0)

#endif /* #if HKU_DISABLE_ASSERT */

/** 抛出 hku::exception 及传入信息 */
#define APP_THROW(...)                                                                           \
    do {                                                                                         \
        throw hku::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                         HKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

/** 抛出指定异常及传入信息 */
#define APP_THROW_EXCEPTION(except, ...)                                                 \
    do {                                                                                 \
        throw except(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                 HKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

//--------------------------------------------------------------
//
// 类 logger 相关宏
//
//--------------------------------------------------------------

#define CLASS_LOGGER(cls)                                                                        \
private:                                                                                         \
    inline static std::shared_ptr<spdlog::logger> ms_##cls_logger;                               \
                                                                                                 \
public:                                                                                          \
    static std::shared_ptr<spdlog::logger> logger() {                                            \
        static std::once_flag oc;                                                                \
        std::call_once(oc, [&]() {                                                               \
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();          \
            stdout_sink->set_level(DEFAULT_LOGGER_LEVEL);                                        \
            ms_##cls_logger = std::make_shared<spdlog::logger>(#cls, stdout_sink);               \
            ms_##cls_logger->set_level(DEFAULT_LOGGER_LEVEL);                                    \
            ms_##cls_logger->flush_on(DEFAULT_LOGGER_LEVEL);                                     \
            ms_##cls_logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [" #cls "-%L] - %v (%s:%#)%$"); \
            spdlog::register_logger(ms_##cls_logger);                                            \
        });                                                                                      \
        return ms_##cls_logger;                                                                  \
    }

#if LOG_ACTIVE_LEVEL <= 0
#define CLS_TRACE(...) SPDLOG_LOGGER_TRACE(logger(), __VA_ARGS__)
#else
#define CLS_TRACE(...)
#endif

#if LOG_ACTIVE_LEVEL <= 1
#define CLS_DEBUG(...) SPDLOG_LOGGER_DEBUG(logger(), __VA_ARGS__)
#else
#define CLS_DEBUG(...)
#endif

#if LOG_ACTIVE_LEVEL <= 2
#define CLS_INFO(...) SPDLOG_LOGGER_INFO(logger(), __VA_ARGS__)
#else
#define CLS_INFO(...)
#endif

#if LOG_ACTIVE_LEVEL <= 3
#define CLS_WARN(...) SPDLOG_LOGGER_WARN(logger(), __VA_ARGS__)
#else
#define CLS_WARN(...)
#endif

#if LOG_ACTIVE_LEVEL <= 4
#define CLS_ERROR(...) SPDLOG_LOGGER_ERROR(logger(), __VA_ARGS__)
#else
#define CLS_ERROR(...)
#endif

#if LOG_ACTIVE_LEVEL <= 5
#define CLS_FATAL(...) SPDLOG_LOGGER_CRITICAL(logger(), __VA_ARGS__)
#else
#define CLS_FATAL(...)
#endif

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

}  // namespace hku