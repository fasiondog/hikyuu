/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <hikyuu/plugin/device.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_device(py::module& m) {
    m.def("bind_email", &bindEmail, R"(bind_email(email: str, code: str)

    绑定邮箱和授权码
    
    :param str email: 邮箱地址
    :param str code: 授权码)");

    m.def("active_device", &activeDevice, py::arg("code"), py::arg("replace") = false,
          R"(active_device(active_code: str)
        
    VIP功能授权码激活设备
    
    :param str code: 授权码
    :param bool replace: 超出设备数量限制时强制替换最早激活设备)");

    m.def("view_license", &viewLicense, R"(view_license()
        
    查看设备授权信息)");

    m.def("remove_license", &removeLicense, R"(remove_license()

    移除当前授权)");

    m.def("fetch_trial_license", &fetchTrialLicense, R"(fetch_trial_license(email: str)

    获取试用授权码
    
    :param str email: 邮箱地址)");

    m.def("is_valid_license", &isValidLicense, R"(is_valid_license()
        
    查看当前设备是否授权)");

    m.def("get_expire_date", &getExpireDate, R"(get_expire_date() -> Datetime
        
    查看授权到期时间)");
}