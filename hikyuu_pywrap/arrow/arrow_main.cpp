/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_arrow_views(py::module& m);

void export_arrow_main(py::module& m) {
    export_arrow_views(m);
}