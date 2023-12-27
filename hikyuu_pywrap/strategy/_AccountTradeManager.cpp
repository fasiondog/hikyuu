/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-28
 *     Author: fasiondog
 */

#include <hikyuu/strategy/AccountTradeManager.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_AccountTradeManger(py::module& m) {
    m.def("crtAccountTM", crtAccountTM);
}