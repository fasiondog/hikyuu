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

    Bind the email and the license code
    
    :param str email: the email address
    :param str code: the license code)");

    m.def("active_device", &activeDevice, py::arg("code"), py::arg("replace") = false,
          R"(active_device(active_code: str)
        
    Activate the device with the donating user license code
    
    :param str code: the license code
    :param bool replace: forcibly replace the earliest activated device when exceeding the device number limit)");

    m.def("view_license", &viewLicense, R"(view_license()
        
    View the device license information)");

    m.def("remove_license", &removeLicense, R"(remove_license()

    Remove the current license)");

    m.def("fetch_trial_license", &fetchTrialLicense, R"(fetch_trial_license(email: str)

    Get the trial license code
    
    :param str email: the email address)");

    m.def("is_valid_license", &isValidLicense, R"(is_valid_license()
        
    Check whether the current device is licensed)");

    m.def("get_expire_date", &getExpireDate, R"(get_expire_date() -> Datetime
        
    View the license expiration time)");
}