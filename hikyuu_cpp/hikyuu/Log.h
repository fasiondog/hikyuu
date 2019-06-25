/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#ifndef HIKUU_LOG_H_
#define HIKUU_LOG_H_

#pragma once

#include "config.h"
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

enum LOG_LEVEL {
    TRACE = SPDLOG_LEVEL_TRACE,
    DEBUG = SPDLOG_LEVEL_DEBUG,
    INFO  = SPDLOG_LEVEL_INFO,
    WARN  = SPDLOG_LEVEL_WARN,
    ERROR = SPDLOG_LEVEL_ERROR,
    FATAL = SPDLOG_LEVEL_CRITICAL,
    OFF   = SPDLOG_LEVEL_OFF,
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

#if HKU_USE_ASYNC_LOGGER
std::shared_ptr<spdlog::async_logger> HKU_API getHikyuuLogger();
#else
std::shared_ptr<spdlog::logger> HKU_API getHikyuuLogger();
#endif /* #if HKU_USE_ASYNC_LOGGER */

/**********************************************
 * Use SPDLOG for logging
 *********************************************/
#define HKU_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define HKU_TRACE(...) SPDLOG_LOGGER_TRACE(getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define HKU_DEBUG(...) SPDLOG_LOGGER_TRACE(getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#define HKU_INFO(...) SPDLOG_LOGGER_INFO(getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#define HKU_WARN(...) SPDLOG_LOGGER_WARN(getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define HKU_ERROR(...) SPDLOG_LOGGER_ERROR(getHikyuuLogger(), __VA_ARGS__)

#define HKU_LOGGER_FATAL(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#define HKU_FATAL(...) SPDLOG_LOGGER_CRITICAL(getHikyuuLogger(), __VA_ARGS__)

} /* namespace hku */
#endif /* HIKUU_LOG_H_ */
