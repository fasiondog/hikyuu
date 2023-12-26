/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <hikyuu/global/GlobalSpotAgent.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_SpotAgent(py::module& m) {
    m.def("start_spot_agent", startSpotAgent, py::arg("print") = false);
    m.def("stop_spot_agent", stopSpotAgent);
}
