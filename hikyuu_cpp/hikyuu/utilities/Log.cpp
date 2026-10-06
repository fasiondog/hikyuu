/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <atomic>
#include <thread>
#include "hikyuu/GlobalInitializer.h"
#include "os.h"
#include "Log.h"

// With stdout_color the log output cannot be redirected to python
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#if HKU_USE_SPDLOG_ASYNC_LOGGER
#include <spdlog/async.h>
#endif /* HKU_USE_SPDLOG_ASYNC_LOGGER */

namespace hku {

// The log level is set and read from arbitrary user threads, so it must be atomic to avoid a
// data race
static std::atomic<LOG_LEVEL> g_log_level{LOG_LEVEL::LOG_TRACE};

LOG_LEVEL get_log_level() {
    return g_log_level.load(std::memory_order_relaxed);
}

void set_log_level(LOG_LEVEL level) {
    g_log_level.store(level, std::memory_order_relaxed);
    getHikyuuLogger()->set_level((spdlog::level::level_enum)level);
}

// Cache the "hikyuu" logger instance to avoid hitting the spdlog registry (a global mutex +
// hash lookup + a shared_ptr copy) on every log call. initLogger registers the logger and
// refreshes the cache; before the first initLogger the default logger is served as fallback.
static std::shared_ptr<spdlog::logger> g_hikyuu_logger;

std::shared_ptr<spdlog::logger> getHikyuuLogger() {
    std::shared_ptr<spdlog::logger> logger =
      std::atomic_load_explicit(&g_hikyuu_logger, std::memory_order_acquire);
    if (!logger) {
        logger = spdlog::default_logger();
        // Concurrent initialization is benign, the last writer wins
        std::atomic_store_explicit(&g_hikyuu_logger, logger, std::memory_order_release);
    }
    return logger;
}

void HKU_UTILS_API initLogger(bool not_use_color, const std::string& filename) {
    std::string logname("hikyuu");
    // Drop the previous registration so a repeated initLogger replaces the logger cleanly
    spdlog::drop(logname);

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

    std::shared_ptr<spdlog::logger> logger;
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
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    std::atomic_store_explicit(&g_hikyuu_logger, logger, std::memory_order_release);
}

}  // namespace hku
