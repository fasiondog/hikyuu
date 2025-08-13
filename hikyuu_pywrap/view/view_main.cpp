/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_MarketView(py::module& m);

void export_view_main(py::module& m) {
    export_MarketView(m);
}