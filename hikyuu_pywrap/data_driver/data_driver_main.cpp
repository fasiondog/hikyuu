/*
 * data_driver_main.cpp
 *
 *  Created on: 2017-10-7
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_DataDriverFactory(py::module& m);
void export_KDataDriver(py::module& m);
// void export_BaseInfoDriver();
void export_BlockInfoDriver(py::module& m);

void export_data_driver_main(py::module& m) {
    // export_BaseInfoDriver();
    export_BlockInfoDriver(m);
    export_KDataDriver(m);
    export_DataDriverFactory(m);
}
