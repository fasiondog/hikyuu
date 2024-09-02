/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include <hikyuu/StrategyContext.h>
#include "pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_StrategeContext(py::module& m) {
    py::class_<StrategyContext>(m, "StrategyContext", "策略上下文")
      .def(py::init<>())
      .def(py::init<const vector<string>&>())
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&>(), py::arg("stock_list"),
           py::arg("ktype_list"))

      .def("__str__", &StrategyContext::str)
      .def("__repr__", &StrategyContext::str)

      .def_property_readonly("start_datetime",
                             py::overload_cast<>(&StrategyContext::startDatetime, py::const_),
                             py::return_value_policy::copy, "起始日期")
      .def_property(
        "stock_list", py::overload_cast<>(&StrategyContext::getStockCodeList, py::const_),
        &StrategyContext::setStockCodeList, py::return_value_policy::copy, "股票代码列表")
      .def_property("ktype_list", py::overload_cast<>(&StrategyContext::getKTypeList, py::const_),
                    &StrategyContext::setKTypeList, py::return_value_policy::copy, "需要的K线类型")

      .def("empty", &StrategyContext::empty, "上下文证券代码列表是否为空");
}
