/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-04
 *      Author: fasiondog
 */

#include <hikyuu/plugin/hkuextra.h>
#include <pybind11/functional.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_hkuextra(py::module& m) {
    m.def("register_ktype_extra", &registerKTypeExtra);
    m.def("is_extra_ktype", &isExtraKType);
    m.def("release_ktype_extra", &releaseKExtra);
}