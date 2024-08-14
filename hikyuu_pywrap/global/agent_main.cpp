/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_SpotRecord(py::module& m);
void export_SpotAgent(py::module& m);

void export_global_main(py::module& m) {
    export_SpotRecord(m);
    export_SpotAgent(m);
}
