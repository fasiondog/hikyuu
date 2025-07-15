/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-13
 *      Author: fasiondog
 */

#include <hikyuu/global/agent/SpotAgent.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_SpotRecord(py::module& m) {
    py::class_<SpotRecord>(m, "SpotRecord")
      .def(py::init<>())
      .def_readwrite("market", &SpotRecord::market)
      .def_readwrite("code", &SpotRecord::code)
      .def_readwrite("name", &SpotRecord::name)
      .def_readwrite("datetime", &SpotRecord::datetime)
      .def_readwrite("yesterday_close", &SpotRecord::yesterday_close)
      .def_readwrite("open", &SpotRecord::open)
      .def_readwrite("high", &SpotRecord::high)
      .def_readwrite("low", &SpotRecord::low)
      .def_readwrite("close", &SpotRecord::close)
      .def_readwrite("amount", &SpotRecord::amount)
      .def_readwrite("volume", &SpotRecord::volume)
      .def_readwrite("bid", &SpotRecord::bid)
      .def_readwrite("bid_amount", &SpotRecord::bid_amount)
      .def_readwrite("ask", &SpotRecord::ask)
      .def_readwrite("ask_amount", &SpotRecord::ask_amount);
}