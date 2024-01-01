/*
 * indicator_main.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_Indicator(py::module& m);
void export_IndicatorImp(py::module& m);
void export_IndParam(py::module& m);
void export_Indicator_build_in(py::module& m);

void export_indicator_main(py::module& m) {
    export_Indicator(m);
    export_IndicatorImp(m);
    export_IndParam(m);
    export_Indicator_build_in(m);
}
