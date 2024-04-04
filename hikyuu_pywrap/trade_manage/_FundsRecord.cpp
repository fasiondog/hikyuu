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
    py::class_<FundsRecord>(m, "FundsRecord", "当前资产情况记录")
      .def(py::init<>())
      .def("__str__", to_py_str<FundsRecord>)
      .def("__repr__", to_py_str<FundsRecord>)

      .def_readwrite("cash", &FundsRecord::cash, "当前现金（float）")
      .def_readwrite("market_value", &FundsRecord::market_value, "当前多头市值（float）")
      .def_readwrite("short_market_value", &FundsRecord::short_market_value,
                     "当前空头仓位市值（float）")
      .def_readwrite("base_cash", &FundsRecord::base_cash, "当前投入本金（float）")
      .def_readwrite("base_asset", &FundsRecord::base_asset, "当前投入的资产价值（float）")
      .def_readwrite("borrow_cash", &FundsRecord::borrow_cash, "当前借入的资金（float），即负债")
      .def_readwrite("borrow_asset", &FundsRecord::borrow_asset, "当前借入证券资产价值（float）")

      .def_property_readonly("total_assets", &FundsRecord::total_assets, "总资产")
      .def_property_readonly("net_assets", &FundsRecord::net_assets, "净资产")
      .def_property_readonly("total_borrow", &FundsRecord::total_borrow, "总负债")
      .def_property_readonly("total_base", &FundsRecord::total_base, "投入本值资产")
      .def_property_readonly("profit", &FundsRecord::profit, "当前收益")

      .def(py::self + py::self)
      .def(py::self += py::self)

        DEF_PICKLE(FundsRecord);
}
