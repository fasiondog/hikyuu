/*
 * _FundsRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/FundsRecord.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_FundsRecord(py::module& m) {
    py::class_<FundsRecord>(m, "FundsRecord", "The current asset situation record")
      .def(py::init<>())
      .def("__str__", to_py_str<FundsRecord>)
      .def("__repr__", to_py_str<FundsRecord>)

      .def_readwrite("cash", &FundsRecord::cash, "The current cash (float)")
      .def_readwrite("market_value", &FundsRecord::market_value, "The current long market value (float)")
      .def_readwrite("short_market_value", &FundsRecord::short_market_value,
                     "The current short position market value (float)")
      .def_readwrite("base_cash", &FundsRecord::base_cash, "The current invested principal (float)")
      .def_readwrite("base_asset", &FundsRecord::base_asset, "The current invested asset value (float)")
      .def_readwrite("borrow_cash", &FundsRecord::borrow_cash, "The currently borrowed funds (float), i.e. the debt")
      .def_readwrite("borrow_asset", &FundsRecord::borrow_asset, "The currently borrowed securities asset value (float)")

      .def_property_readonly("total_assets", &FundsRecord::total_assets, "The total assets")
      .def_property_readonly("net_assets", &FundsRecord::net_assets, "The net assets")
      .def_property_readonly("total_borrow", &FundsRecord::total_borrow, "The total debt")
      .def_property_readonly("total_base", &FundsRecord::total_base, "The invested principal assets")
      .def_property_readonly("profit", &FundsRecord::profit, "The current profit")

      .def(py::self + py::self)
      .def(py::self += py::self)

        DEF_PICKLE(FundsRecord);
}
