/*
 * instance_main.cpp
 *
 *  Created on: 2015年3月14日
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_AmaInstance(py::module& m);

void export_instance_main(py::module& m) {
    export_AmaInstance(m);
}
