/*
 * ioredirect.cpp
 *
 *  Created on: 2018年8月27日
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>
#include "ioredirect.h"

namespace py = pybind11;

void export_io_redirect(py::module& m) {
    py::class_<OstreamRedirect>(m, "OstreamRedirect")
      .def(py::init<bool, bool>(), py::arg("stdout") = true, py::arg("stderr") = true)
      .def("__enter__", &OstreamRedirect::enter)
      .def("__exit__", &OstreamRedirect::exit)
      .def("open", &OstreamRedirect::enter)
      .def("close", &OstreamRedirect::exit);
}
