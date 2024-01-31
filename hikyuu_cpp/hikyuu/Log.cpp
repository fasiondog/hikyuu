/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <thread>
#include "config.h"
#include "GlobalInitializer.h"
#include "Log.h"

#if USE_SPDLOG_LOGGER
// 使用 stdout_color 将无法将日志输出重定向至 python
// #include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include "spdlog/sinks/ostream_sink.h"
// #include "spdlog/sinks/rotating_file_sink.h"

#if HKU_USE_SPDLOG_ASYNC_LOGGER
#include <spdlog/async.h>
#endif /* HKU_USE_SPDLOG_ASYNC_LOGGER */
#endif /* #if USE_SPDLOG_LOGGER */

namespace hku {

static std::thread::id g_main_thread_id = std::this_thread::get_id();
static int g_ioredirect_to_python_count = 0;

bool isLogInMainThread() {
    return std::this_thread::get_id() == g_main_thread_id;
}

int getIORedirectToPythonCount() {
    return g_ioredirect_to_python_count;
}

void increaseIORedicrectToPythonCount() {
    g_ioredirect_to_python_count++;
}

void decreaseIORedicrectToPythonCount() {
    g_ioredirect_to_python_count--;
}

static LOG_LEVEL g_log_level = LOG_LEVEL::LOG_TRACE;

std::string g_unknown_error_msg{"Unknown error!"};

LOG_LEVEL get_log_level() {
    return g_log_level;
}

/**********************************************
 * Use SPDLOG for logging
 *********************************************/
#if USE_SPDLOG_LOGGER
std::shared_ptr<spdlog::logger> getHikyuuLogger() {
    return spdlog::get("hikyuu");
}

#if HKU_USE_SPDLOG_ASYNC_LOGGER
void initLogger() {
    auto stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
    // auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::trace);

    spdlog::init_thread_pool(8192, 1);
    std::vector<spdlog::sink_ptr> sinks{stdout_sink};
    auto logger = std::make_shared<spdlog::async_logger>("hikyuu", sinks.begin(), sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v [%!]");
    spdlog::register_logger(logger);
}

#else /* #if HKU_USE_SPDLOG_ASYNC_LOGGER */

void initLogger() {
    auto stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
    // auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::trace);
    auto logger = std::make_shared<spdlog::logger>("hikyuu", stdout_sink);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    // logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v [%!] (%@)");
    // logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v (%@)");
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v (%s:%#)");
    spdlog::register_logger(logger);
}

#endif /* #if HKU_USE_SPDLOG_ASYNC_LOGGER */

void set_log_level(LOG_LEVEL level) {
    g_log_level = level;
    getHikyuuLogger()->set_level((spdlog::level::level_enum)level);
}

#else /* #if USE_SPDLOG_LOGGER */
/**********************************************
 * Use SPDLOG for logging
 *********************************************/
void initLogger() {}

void set_log_level(LOG_LEVEL level) {
    g_log_level = level;
}

std::string HKU_API getLocalTime() {
    auto now = std::chrono::system_clock::now();
    uint64_t dis_millseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() -
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
    time_t tt = std::chrono::system_clock::to_time_t(now);
#ifdef _WIN32
    struct tm now_time;
    localtime_s(&now_time, &tt);
#else
    struct tm now_time;
    localtime_r(&tt, &now_time);
#endif
    return fmt::format("{:%Y-%m-%d %H:%M:%S}.{:<3d}", now_time, dis_millseconds);
}

#endif /* #if USE_SPDLOG_LOGGER */

}  // namespace hku
