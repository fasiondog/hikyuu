/*
 * trade_sys_main.cpp
 *
 *  Created on: 2013-3-2
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_Environment(py::module& m);
void export_Condition(py::module& m);
void export_MoneyManager(py::module& m);
void export_Signal(py::module& m);
void export_Stoploss(py::module& m);
void export_ProfitGoal(py::module& m);
void export_Slippage(py::module& m);
void export_System(py::module& m);
void export_Selector(py::module& m);
void export_Portfolio(py::module& m);
void export_AllocateFunds(py::module& m);

void export_trade_sys_main(py::module& m) {
    export_Environment(m);
    export_Condition(m);
    export_MoneyManager(m);
    export_Signal(m);
    export_Stoploss(m);
    export_ProfitGoal(m);
    export_Slippage(m);
    export_System(m);
    export_Selector(m);
    export_AllocateFunds(m);
    export_Portfolio(m);
}
