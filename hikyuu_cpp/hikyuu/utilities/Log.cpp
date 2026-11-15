/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
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

// The HKU_* macros hit this on every log call, so the read must stay lock-free and refcount-free:
// a single acquire load of a raw pointer. Only initLogger (startup, rare) publishes a new logger,
// and it takes the mutex. Pointers stay valid for the process lifetime because every published
// logger is kept in state.retired: a reader that loaded the old pointer may still be inside spdlog
// while a re-init replaces it, and C++ background threads may still log during exit.
static std::atomic<spdlog::logger*> g_logger_ptr{nullptr};

struct LoggerState {
    std::mutex mutex;
    std::shared_ptr<spdlog::logger> current;
    // initLogger is called only a few times, so keeping all instances is cheap
    std::vector<std::shared_ptr<spdlog::logger>> retired;
};

// Deliberately leaked: Python's atexit cleanup may destroy spdlog globals while C++ background
// threads are still logging.
static LoggerState& loggerState() {
    static auto* state = new LoggerState();
    return *state;
}

// Callers must hold loggerState().mutex.
static void publishLogger(const std::shared_ptr<spdlog::logger>& logger) {
    LoggerState& state = loggerState();
    state.retired.push_back(logger);
    state.current = logger;
    g_logger_ptr.store(logger.get(), std::memory_order_release);
}

HKU_UTILS_API spdlog::logger* getHikyuuLoggerRaw() {
    spdlog::logger* logger = g_logger_ptr.load(std::memory_order_acquire);
    if (logger) {
        return logger;
    }

    // Before the first initLogger, spdlog's default logger is the fallback. Fetch it before
    // taking our own mutex, then publish only if nobody else did in the meantime.
    auto fallback = spdlog::default_logger();
    LoggerState& state = loggerState();
    std::lock_guard<std::mutex> lock(state.mutex);
    if (!g_logger_ptr.load(std::memory_order_relaxed)) {
        publishLogger(fallback);
    }
    return g_logger_ptr.load(std::memory_order_acquire);
}

std::shared_ptr<spdlog::logger> getHikyuuLogger() {
    // Cold path: an owning handle keeps the logger alive across a concurrent re-init.
    getHikyuuLoggerRaw();
    LoggerState& state = loggerState();
    std::lock_guard<std::mutex> lock(state.mutex);
    return state.current;
}

HKU_UTILS_API void initLogger(bool not_use_color, const std::string& filename) {
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
    LoggerState& state = loggerState();
    std::lock_guard<std::mutex> lock(state.mutex);
    publishLogger(logger);
}

}  // namespace hku
