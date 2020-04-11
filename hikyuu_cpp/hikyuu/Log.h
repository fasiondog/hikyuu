/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKUU_LOG_H_
#define HIKUU_LOG_H_

#include "config.h"

// clang-format off
#if USE_SPDLOG_LOGGER
    #include <spdlog/spdlog.h>
    #include <spdlog/fmt/ostr.h>
    #if HKU_USE_SPDLOG_ASYNC_LOGGER
        #include "spdlog/async.h"
    #endif
#endif
// clang-format on

#include <fmt/ostream.h>
#include <fmt/format.h>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * @ingroup Utilities
 * @addtogroup logging Logging tools 日志工具
 * @details 打印等级：
 * TRACE < DEBUG < INFO < WARN < ERROR < FATAL
 * @{
 */

/**********************************************
 * Use SPDLOG for logging
 *********************************************/
#if USE_SPDLOG_LOGGER
/** 日志级别 */
enum LOG_LEVEL {
    TRACE = SPDLOG_LEVEL_TRACE,     ///< 跟踪
    DEBUG = SPDLOG_LEVEL_DEBUG,     ///< 调试
    INFO = SPDLOG_LEVEL_INFO,       ///< 一般信息
    WARN = SPDLOG_LEVEL_WARN,       ///< 告警
    ERROR = SPDLOG_LEVEL_ERROR,     ///< 错误
    FATAL = SPDLOG_LEVEL_CRITICAL,  ///< 致命
    OFF = SPDLOG_LEVEL_OFF,         ///< 关闭日志打印
};

/**
 * 获取当前日志级别
 * @return
 */
LOG_LEVEL HKU_API get_log_level();

/**
 * 设置日志级别
 * @param level 指定的日志级别
 */
void HKU_API set_log_level(LOG_LEVEL level);

std::shared_ptr<spdlog::logger> HKU_API getHikyuuLogger();

#define HKU_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define HKU_TRACE(...) SPDLOG_LOGGER_TRACE(hku::getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define HKU_DEBUG(...) SPDLOG_LOGGER_TRACE(hku::getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#define HKU_INFO(...) SPDLOG_LOGGER_INFO(hku::getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#define HKU_WARN(...) SPDLOG_LOGGER_WARN(hku::getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define HKU_ERROR(...) SPDLOG_LOGGER_ERROR(hku::getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_FATAL(logger, ...) \
    SPDLOG_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#define HKU_FATAL(...) SPDLOG_LOGGER_CRITICAL(hku::getHikyuuLogger(), __VA_ARGS__)

#if HKU_USE_SPDLOG_ASYNC_LOGGER
void init_logger();
#else
void init_logger();
#endif

#else
enum LOG_LEVEL {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    OFF = 6,
};

LOG_LEVEL HKU_API get_log_level();
void HKU_API set_log_level(LOG_LEVEL level);
void init_logger();

#if LOG_ACTIVE_LEVEL <= 0
#define HKU_TRACE(...) \
    fmt::print("[HKU-T] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_TRACE(...)
#endif

#if LOG_ACTIVE_LEVEL <= 1
#define HKU_DEBUG(...) \
    fmt::print("[HKU-D] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_DEBUG(...)
#endif

#if LOG_ACTIVE_LEVEL <= 2
#define HKU_INFO(...) \
    fmt::print("[HKU-I] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_INFO(...)
#endif

#if LOG_ACTIVE_LEVEL <= 3
#define HKU_WARN(...) \
    fmt::print("[HKU-W] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_WARN(...)
#endif

#if LOG_ACTIVE_LEVEL <= 4
#define HKU_ERROR(...) \
    fmt::print("[HKU-E] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_ERROR(...)
#endif

#if LOG_ACTIVE_LEVEL <= 5
#define HKU_FATAL(...) \
    fmt::print("[HKU-F] - {} ({}:{})\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#else
#define HKU_FATAL(...)
#endif

#endif /* USE_SPDLOG_LOGGER */

/** @} */

} /* namespace hku */

#endif /* HIKUU_LOG_H_ */
