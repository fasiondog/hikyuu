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
    py::class_<StrategyContext>(m, "StrategyContext", "The strategy context")
      .def(py::init<>())
      .def(py::init<const vector<string>&>())
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&,
                    const unordered_map<string, int64_t>&>(),
           py::arg("stock_list"), py::arg("ktype_list"),
           py::arg("preload_num") = unordered_map<string, int64_t>(),
           R"(__init__(self, stock_list, ktype_list, [preload_num={}])
        
  Create the strategy context

  :param stock_list: the security code list to load, e.g.: ["sz000001", "sz000002"]; if it contains 'ALL', it means loading all
  :param ktype_list: the K-line type list to load, e.g.: ["day", "min"]; when unspecified, take the default value configured in the global configuration file
  :param preload_num: the preloading quantity, defaulting to empty, e.g.: {"min_max": 100, "day_max": 200}. When unspecified, take the default value configured in the global configuration file
  :return: the strategy context object)")

      .def("__str__", &StrategyContext::str)
      .def("__repr__", &StrategyContext::str)

      .def_property_readonly("start_datetime",
                             py::overload_cast<>(&StrategyContext::startDatetime, py::const_),
                             py::return_value_policy::copy, "The start date")
      .def_property(
        "stock_list", py::overload_cast<>(&StrategyContext::getStockCodeList, py::const_),
        &StrategyContext::setStockCodeList, py::return_value_policy::copy, "The stock code list")
      .def_property("ktype_list", py::overload_cast<>(&StrategyContext::getKTypeList, py::const_),
                    &StrategyContext::setKTypeList, py::return_value_policy::copy, "The needed K-line types")
      .def_property("preload_num", py::overload_cast<>(&StrategyContext::getPreloadNum, py::const_),
                    &StrategyContext::setPreloadNum, py::return_value_policy::copy, "The preloading quantity")

      .def("empty", &StrategyContext::empty, "Whether the security code list in the context is empty");
}
