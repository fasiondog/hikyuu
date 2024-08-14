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
      .def_readwrite("bid1", &SpotRecord::bid1)
      .def_readwrite("bid1_amount", &SpotRecord::bid1_amount)
      .def_readwrite("bid2", &SpotRecord::bid2)
      .def_readwrite("bid2_amount", &SpotRecord::bid2_amount)
      .def_readwrite("bid3", &SpotRecord::bid3)
      .def_readwrite("bid3_amount", &SpotRecord::bid3_amount)
      .def_readwrite("bid4", &SpotRecord::bid4)
      .def_readwrite("bid4_amount", &SpotRecord::bid4_amount)
      .def_readwrite("bid5", &SpotRecord::bid5)
      .def_readwrite("bid5_amount", &SpotRecord::bid5_amount)
      .def_readwrite("ask1", &SpotRecord::ask1)
      .def_readwrite("ask1_amount", &SpotRecord::ask1_amount)
      .def_readwrite("ask2", &SpotRecord::ask2)
      .def_readwrite("ask2_amount", &SpotRecord::ask2_amount)
      .def_readwrite("ask3", &SpotRecord::ask3)
      .def_readwrite("ask3_amount", &SpotRecord::ask3_amount)
      .def_readwrite("ask4", &SpotRecord::ask4)
      .def_readwrite("ask4_amount", &SpotRecord::ask4_amount)
      .def_readwrite("ask5", &SpotRecord::ask5)
      .def_readwrite("ask5_amount", &SpotRecord::ask5_amount);
}