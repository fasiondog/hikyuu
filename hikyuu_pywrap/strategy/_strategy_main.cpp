/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_Strategy(py::module& m);

void export_strategy_main(py::module& m) {
    export_Strategy(m);
}