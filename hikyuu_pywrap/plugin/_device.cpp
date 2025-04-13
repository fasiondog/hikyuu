/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <csignal>
#include <hikyuu/plugin/device.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_device(py::module& m) {
    m.def("activate_device", activateDevice, R"(activate_device(active_code: str)
        
    VIP功能授权码激活设备
    
    :param str active_code: 授权码)");

    m.def("view_license", viewLicense, R"(view_license()
        
    查看设备授权信息)");
}