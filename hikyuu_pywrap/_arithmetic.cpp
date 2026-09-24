/*
 * _util.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <hikyuu/utilities/arithmetic.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_util(py::module& m) {
    m.def("roundEx", roundEx<float>, py::arg("number"), py::arg("ndigits") = 0);
    m.def("roundEx", roundEx<double>, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundEx(number[, ndigits=0])

    Round half up, with the ROUND_HALF_EVEN banker's rounding

    :param float number  the data to round
    :param int ndigits the number of the decimal places to keep
    :rype: float)");

    m.def("roundUp", roundUp<float>, py::arg("number"), py::arg("ndigits") = 0);
    m.def("roundUp", roundUp<double>, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundUp(number[, ndigits=0])

    Round up, e.g. 10.1 becomes 11 after rounding

    :param float number  the data to process
    :param int ndigits the number of the decimal places to keep
    :rtype: float)");

    m.def("roundDown", roundDown<float>, py::arg("number"), py::arg("ndigits") = 0);
    m.def("roundDown", roundDown<double>, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundDown(number[, ndigits=0])

    Round down, e.g. 10.1 becomes 10 after rounding

    :param float number  the data to process
    :param int ndigits the number of the decimal places to keep
    :rtype: float)");
}
