/*
 * _AmaInstance.cpp
 *
 *  Created on: 2015年3月14日
 *      Author: fasiondog
 */

#include <hikyuu/trade_instance/ama_sys/AmaInstance.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace hku;

void export_AmaInstance(py::module& m) {
    m.def("AmaSpecial", AmaSpecial);
}
