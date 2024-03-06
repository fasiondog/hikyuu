/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <hikyuu/Log.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_log(py::module& m) {
    py::enum_<LOG_LEVEL>(m, "LOG_LEVEL")
      .value("DEBUG", LOG_LEVEL::LOG_DEBUG)
      .value("TRACE", LOG_LEVEL::LOG_TRACE)
      .value("INFO", LOG_LEVEL::LOG_INFO)
      .value("WARN", LOG_LEVEL::LOG_WARN)
      .value("ERROR", LOG_LEVEL::LOG_ERROR)
      .value("FATAL", LOG_LEVEL::LOG_FATAL)
      .value("OFF", LOG_LEVEL::LOG_OFF)
      .export_values();

    m.def("get_log_level", get_log_level, "获取当前日志级别");
    m.def("set_log_level", set_log_level, "设置当前日志级别");
}
