/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-05
 *     Author: fasiondog
 */

#pragma once

#include <iostream>
#include <hikyuu/log.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>

#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

namespace hku {

inline void init_server_logger() {
    static std::once_flag oc;
    std::call_once(oc, [&]() {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(spdlog::level::trace);

        spdlog::init_thread_pool(8192, 1);
        std::vector<spdlog::sink_ptr> sinks{stdout_sink};
        auto logger = std::make_shared<spdlog::async_logger>("SERVER", sinks.begin(), sinks.end(),
                                                             spdlog::thread_pool(),
                                                             spdlog::async_overflow_policy::block);

        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^SERVER-%L%$] - %v (%s:%#)");
        spdlog::set_default_logger(logger);
    });
}

#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

/**
 * 满足指定条件时，打印 TRACE 信息
 * @param expr 指定条件
 */
#define LOG_TRACE_IF(expr, ...) \
    if (expr) {                 \
        LOG_TRACE(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 */
#define LOG_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        LOG_DEBUG(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 */
#define LOG_INFO_IF(expr, ...) \
    if (expr) {                \
        LOG_INFO(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 */
#define LOG_WARN_IF(expr, ...) \
    if (expr) {                \
        LOG_WARN(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 */
#define LOG_ERROR_IF(expr, ...) \
    if (expr) {                 \
        LOG_ERROR(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 */
#define LOG_FATAL_IF(expr, ...) \
    if (expr) {                 \
        LOG_FATAL(__VA_ARGS__); \
    }

/**
 * 满足指定条件时，返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * 满足指定条件时，打印 TRACE 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        LOG_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 DEBUG 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        LOG_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 INFO 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        LOG_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 WARN 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        LOG_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * 满足指定条件时，打印 ERROR 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        LOG_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * 满足指定条件时，打印 FATAL 信息, 并返回指定值
 * @param expr 指定条件
 * @param ret 返回值
 */
#define LOG_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        LOG_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

//--------------------------------------------------------------
//
// 类 logger 相关宏
//
//--------------------------------------------------------------

#define CLASS_LOGGER(cls)                                                                        \
private:                                                                                         \
    inline static std::shared_ptr<spdlog::async_logger> ms_##cls_logger;                         \
                                                                                                 \
public:                                                                                          \
    static std::shared_ptr<spdlog::async_logger> logger() {                                      \
        static std::once_flag oc;                                                                \
        std::call_once(oc, [&]() {                                                               \
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();          \
            stdout_sink->set_level(spdlog::level::trace);                                        \
                                                                                                 \
            spdlog::init_thread_pool(8192, 1);                                                   \
            std::vector<spdlog::sink_ptr> sinks{stdout_sink};                                    \
            ms_##cls_logger = std::make_shared<spdlog::async_logger>(                            \
              #cls, sinks.begin(), sinks.end(), spdlog::thread_pool(),                           \
              spdlog::async_overflow_policy::block);                                             \
                                                                                                 \
            ms_##cls_logger->set_level(spdlog::level::trace);                                    \
            ms_##cls_logger->flush_on(spdlog::level::trace);                                     \
            ms_##cls_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^" #cls "-%L%$] - %v (%s:%#)"); \
            spdlog::register_logger(ms_##cls_logger);                                            \
        });                                                                                      \
        return ms_##cls_logger;                                                                  \
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

}  // namespace hku