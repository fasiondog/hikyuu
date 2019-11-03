/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <iostream>
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
//#include "spdlog/sinks/ostream_sink.h"
//#include "spdlog/sinks/rotating_file_sink.h"

#if HKU_USE_ASYNC_LOGGER
#include <spdlog/async.h>
#endif /* HKU_USE_ASYNC_LOGGER */

namespace hku {

namespace inner {
    
    static LOG_LEVEL g_log_level = TRACE;

    /**********************************************
     * Use SPDLOG for logging
     *********************************************/
    #if HKU_USE_ASYNC_LOGGER
    std::shared_ptr<spdlog::async_logger> init_logger() {
        //auto stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(spdlog::level::trace);
        
        spdlog::init_thread_pool(8192, 1);
        std::vector<spdlog::sink_ptr> sinks {stdout_sink};
        auto logger = std::make_shared<spdlog::async_logger>("hikyuu", 
            sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        //logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v [%!]");
        logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v [%!]");
        spdlog::register_logger(logger);
        return logger;
    }

    std::shared_ptr<spdlog::async_logger> g_hikyuu_logger;

    std::shared_ptr<spdlog::async_logger> getHikyuuLogger() {
        return g_hikyuu_logger;
    }
    #else

    std::shared_ptr<spdlog::logger> init_logger() {
        //auto stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(spdlog::level::trace);
        
        auto logger = std::shared_ptr<spdlog::logger>(
                new spdlog::logger("hikyuu", stdout_sink));

        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        //logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v [%!]");
        logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HKU-%L%$] - %v [%!]");
        spdlog::register_logger(logger);
        return logger;
    }

    std::shared_ptr<spdlog::logger> g_hikyuu_logger;
} /*namespace inner */

LOG_LEVEL get_log_level() {
    return inner::g_log_level;
}

void set_log_level(LOG_LEVEL level) {
    inner::g_log_level = level;
    inner::g_hikyuu_logger->set_level((spdlog::level::level_enum)level);
}

std::shared_ptr<spdlog::logger> HKU_API getHikyuuLogger() {
    return inner::g_hikyuu_logger;
}

#endif /* #if HKU_USE_ASYNC_LOGGER */

} /* namespace hku */
