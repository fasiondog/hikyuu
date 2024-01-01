/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-07
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_IndParam(py::module& m) {
    py::class_<IndParam>(m, "IndParam", "技术指标")
      .def(py::init<>())
      .def(py::init<IndicatorImpPtr>())
      .def(py::init<Indicator>())
      .def("__str__", to_py_str<IndParam>)
      .def("__repr__", to_py_str<IndParam>)

      .def("get", &IndParam::get)
      .def("get_imp", &IndParam::getImp);
}