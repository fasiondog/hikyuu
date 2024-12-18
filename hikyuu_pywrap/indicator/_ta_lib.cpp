/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator/build_in.h>

#if HKU_ENABLE_TA_LIB

#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Indicator_ta_lib(py::module& m) {
    m.def("TA_CMO", py::overload_cast<int>(TA_CMO), py::arg("n") = 14);
    m.def("TA_CMO", py::overload_cast<const Indicator&, int>(TA_CMO), py::arg("data"),
          py::arg("n") = 14, R"(TA_CMO(data, n=14) -> Indicator)

    Chande Momentum Oscillator

    :param Indicator data: 指定指标
    :param n: Number of period (From 2 to 100000))");
}

#endif /* HKU_ENABLE_TA_LIB */