/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <thread>
#include "hikyuu/GlobalInitializer.h"
#include "os.h"
#include "Log.h"

// 使用 stdout_color 将无法将日志输出重定向至 python
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#if HKU_USE_SPDLOG_ASYNC_LOGGER
#include <spdlog/async.h>
#endif /* HKU_USE_SPDLOG_ASYNC_LOGGER */

namespace hku {

static LOG_LEVEL g_log_level = LOG_LEVEL::LOG_TRACE;

std::string g_unknown_error_msg{"Unknown error!"};

LOG_LEVEL get_log_level() {
    return g_log_level;
}

void set_log_level(LOG_LEVEL level) {
    g_log_level = level;
    getHikyuuLogger()->set_level((spdlog::level::level_enum)level);
}

std::shared_ptr<spdlog::logger> getHikyuuLogger() {
    auto logger = spdlog::get("hikyuu");
    return logger ? logger : spdlog::default_logger();
}

void HKU_UTILS_API initLogger(bool not_use_color, const std::string& filename) {
    std::string logname("hikyuu");
    spdlog::drop(logname);
    std::shared_ptr<spdlog::logger> logger = spdlog::get(logname);
    if (logger) {
        spdlog::drop(logname);
    }

    spdlog::sink_ptr stdout_sink;
    if (not_use_color) {
        stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
    } else {
        stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    stdout_sink->set_level(spdlog::level::trace);

    std::string logfile = filename.empty() ? "./hikyuu.log" : filename;
    auto rotating_sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfile, 1024 * 1024 * 10, 3);
    rotating_sink->set_level(spdlog::level::warn);

    std::vector<spdlog::sink_ptr> sinks{stdout_sink};
    if (rotating_sink) {
        sinks.emplace_back(rotating_sink);
    }

#if HKU_USE_SPDLOG_ASYNC_LOGGER
    spdlog::init_thread_pool(8192, 1);
    logger = std::make_shared<spdlog::async_logger>(logname, sinks.begin(), sinks.end(),
                                                    spdlog::thread_pool(),
                                                    spdlog::async_overflow_policy::block);
#else
    logger = std::make_shared<spdlog::logger>(logname, sinks.begin(), sinks.end());
#endif

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v (%s:%#)");
    // logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [HKU-%L] - %v (%s:%#)%$");
    // spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

}  // namespace hku
