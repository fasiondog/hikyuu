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
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&>(), py::arg("stock_list"),
           py::arg("ktype_list"))
      .def_property_readonly("start_datetime",
                             py::overload_cast<>(&StrategyContext::startDatetime, py::const_),
                             "起始日期")
      .def_property(
        "stock_list", py::overload_cast<>(&StrategyContext::getStockCodeList, py::const_),
        [](StrategyContext& self, const py::sequence& stk_list) {
            self.setStockCodeList(python_list_to_vector<string>(stk_list));
        },
        "股票代码列表")
      .def_property(
        "ktype_list", py::overload_cast<>(&StrategyContext::getKTypeList, py::const_),
        [](StrategyContext& self, const py::sequence& ktype_list) {
            self.setKTypeList(python_list_to_vector<string>(ktype_list));
        },
        "需要的K线类型");
}
