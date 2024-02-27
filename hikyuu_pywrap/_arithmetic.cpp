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
    m.def("roundEx", roundEx<double>, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundEx(number[, ndigits=0])

    四舍五入，ROUND_HALF_EVEN 银行家舍入法

    :param float number  待四舍五入的数据
    :param int ndigits 保留小数位数
    :rype: float)");

    m.def("roundEx", roundEx<float>, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundEx(number[, ndigits=0])

    四舍五入，ROUND_HALF_EVEN 银行家舍入法

    :param float number  待四舍五入的数据
    :param int ndigits 保留小数位数
    :rype: float)");

    m.def("roundUp", roundUp, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundUp(number[, ndigits=0])

    向上截取，如10.1截取后为11

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");

    m.def("roundDown", roundDown, py::arg("number"), py::arg("ndigits") = 0,
          R"(roundDown(number[, ndigits=0])

    向下截取，如10.1截取后为10

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");
}
