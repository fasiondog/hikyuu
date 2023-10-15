/*
 * _util.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/utilities/arithmetic.h>
#include "pybind_utils.h"

using namespace boost::python;
using namespace hku;

namespace py = boost::python;

using namespace hku;

void export_util() {
    def("roundEx", roundEx, (arg("number"), arg("ndigits") = 0),
        R"(roundEx(number[, ndigits=0])

    四舍五入，ROUND_HALF_EVEN 银行家舍入法

    :param float number  待四舍五入的数据
    :param int ndigits 保留小数位数
    :rype: float)");

    def("roundUp", roundUp, (arg("number"), arg("ndigits") = 0), R"(roundUp(number[, ndigits=0])

    向上截取，如10.1截取后为11

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");

    def("roundDown", roundDown, (arg("number"), arg("ndigits") = 0),
        R"(roundDown(number[, ndigits=0])

    向下截取，如10.1截取后为10

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");
}
