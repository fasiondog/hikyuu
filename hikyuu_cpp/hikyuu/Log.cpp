/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <iostream>
#include "Log.h"

#ifdef USE_BOOST_LOG_FOR_LOGGING
namespace logging = boost::log;
#endif

#ifdef USE_SPDLOG_FOR_LOGGING
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#endif

namespace hku {

static LOG_LEVEL g_log_level = TRACE;

LOG_LEVEL get_log_level() {
    return g_log_level;
}

void set_log_level(LOG_LEVEL level) {
    g_log_level = level;
}

/**********************************************
 * Use Boost.log for logging
 *
 *********************************************/
#ifdef USE_BOOST_LOG_FOR_LOGGING
void init_logger(const std::string& configure_name) {
    logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info
            );
}
#endif /* for USE_BOOST_LOG_FOR_LOGGING */


/**********************************************
 * Use SPDLOG for logging
 *
 *********************************************/
#ifdef USE_SPDLOG_FOR_LOGGING
void init_logger(const std::string& configure_name) {
    // Create a file rotating sink with 5mb size max and 3 rotated files
    // Writing to log file can't be shared, multiple program run will fail.
    //auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("hikyuu.log", 1048576 * 5, 3);
    //file_sink->set_level(spdlog::level::trace);

    // Create color console sink for server console
    //auto server_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //server_sink->set_level(spdlog::level::trace);

    // Create std::cout sink, python app(like Jupyter notebook) will catch redirect io.
    auto stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
    stdout_sink->set_level(spdlog::level::trace);

    auto logger = std::shared_ptr<spdlog::logger>(
            //new spdlog::logger("hikyuu", {stdout_sink, file_sink}));
            new spdlog::logger("hikyuu", stdout_sink));
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::details::registry::instance().register_logger(logger);
}
#endif


/**********************************************
 * Use STDOUT for logging, not threadsafe
 *
 *********************************************/
#ifdef USE_STDOUT_FOR_LOGGING
void init_logger(const std::string& configure_name) {

}
#endif /* for USE_STDOUT_FOR_LOGGING */

} /* namespace hku */
