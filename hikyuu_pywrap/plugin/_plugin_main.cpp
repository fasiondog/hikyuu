/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_plugin_device(py::module& m);
void export_plugin_backtest(py::module& m);
void export_plugin_dataserver(py::module& m);
void export_plugin_KDataToHdf5Importer(py::module& m);
void export_extend_Indicator(py::module& m);
void export_hkuextra(py::module& m);

void export_plugin(py::module& m) {
    export_plugin_device(m);
    export_plugin_backtest(m);
    export_plugin_dataserver(m);
    export_plugin_KDataToHdf5Importer(m);
    export_extend_Indicator(m);
    export_hkuextra(m);
}