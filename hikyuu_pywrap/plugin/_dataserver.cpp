/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <csignal>
#include <hikyuu/plugin/dataserver.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_dataserver(py::module& m) {
    m.def("start_data_server", startDataServer, py::arg("addr") = "tcp://0.0.0.0:9201",
          py::arg("work_num") = 2, R"(start_data_server(addr: str[, work_num: int=2])
        
    启动数据缓存服务
    
    :param str addr: 服务器地址
    :param int work_num: 工作线程数
    :return: None)");

    m.def("stop_data_server", stopDataServer, R"(stop_data_server()
        
    停止数据缓存服务)");
}