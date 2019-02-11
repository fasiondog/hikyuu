/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/Log.h>

using namespace boost::python;
using namespace hku;

void export_log() {
    enum_<LOG_LEVEL>("LOG_LEVEL")
            .value("DEBUG", LOG_LEVEL::DEBUG)
            .value("TRACE", LOG_LEVEL::TRACE)
            .value("INFO", LOG_LEVEL::INFO)
            .value("WARN", LOG_LEVEL::WARN)
            .value("ERROR", LOG_LEVEL::ERROR)
            .value("FATAL", LOG_LEVEL::FATAL)
            .value("NO_PRINT", LOG_LEVEL::NO_PRINT)
            ;

    def("get_log_level", get_log_level);
    def("set_log_level", set_log_level);
}


