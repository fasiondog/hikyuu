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
    m.def("active_device", activeDevice, R"(active_device(active_code: str)
        
    VIP功能授权码激活设备
    
    :param str active_code: 授权码)");

    m.def("view_license", viewLicense, R"(view_license()
        
    查看设备授权信息)");

    m.def("remove_license", removeLicense, R"(remove_license()
        
    移除当前授权)");

    m.def("fetch_trial_license", fetchTrialLicense, R"(fetch_trial_license(email: str)
        
    获取试用授权码
    
    :param str email: 邮箱地址)");
}