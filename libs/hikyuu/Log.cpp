/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include "Log.h"

using namespace boost::filesystem;
using namespace log4cplus;

namespace hku {

void init_logger(const std::string& configure_name) {
    bool use_configue = false;
    if (configure_name != "") {
        path p(configure_name);
        if(exists(p) && !is_directory(p)) {
            PropertyConfigurator::doConfigure(configure_name);
            use_configue = true;
        }
    }

    if (!use_configue) {
        SharedAppenderPtr _append(new ConsoleAppender());
        //std::auto_ptr<Layout> _layout(new SimpleLayout());
        //std::auto_ptr<Layout> _layout(new TTCCLayout(true));
        std::auto_ptr<Layout> _layout(new PatternLayout("%d{%H:%M:%S} [%c] %p - %m%n"));
        _append->setLayout( _layout );
        Logger::getRoot().addAppender(_append);
    }
}

log4cplus::Logger getLogger() {
    return Logger::getInstance("hikyuu");
}


} /* namespace hku */
