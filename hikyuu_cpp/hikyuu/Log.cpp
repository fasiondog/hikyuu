/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include "Log.h"

using namespace boost::filesystem;

#ifdef USE_BOOST_LOG_FOR_LOGGING
namespace logging = boost::log;
#endif

namespace hku {


/**********************************************
 * Use log4cplus for logging
 *
 *********************************************/
#ifdef USE_LOG4CPLUS_FOR_LOGGING
void init_logger(const std::string& configure_name) {
    bool use_configue = false;
    if (configure_name != "") {
        path p(configure_name);
        if(exists(p) && !is_directory(p)) {
            log4cplus::PropertyConfigurator::doConfigure(configure_name);
            use_configue = true;
        }
    }

    if (!use_configue) {
        log4cplus::SharedAppenderPtr _append(new log4cplus::ConsoleAppender());
        //std::auto_ptr<Layout> _layout(new SimpleLayout());
        //std::auto_ptr<Layout> _layout(new TTCCLayout(true));
        std::auto_ptr<log4cplus::Layout> _layout(
                new log4cplus::PatternLayout("%d{%H:%M:%S} [%c] %p - %m%n")
        );
        _append->setLayout( _layout );
        log4cplus::Logger::getRoot().addAppender(_append);
    }
}

log4cplus::Logger getLogger() {
    return Logger::getInstance("hikyuu");
}
#endif /* for USE_LOG4CPLUS_FOR_LOGGING */


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
 * Use STDOUT for logging, not threadsafe
 *
 *********************************************/
#ifdef USE_STDOUT_FOR_LOGGING
void init_logger(const std::string& configure_name) {

}
#endif /* for USE_STDOUT_FOR_LOGGING */



} /* namespace hku */
