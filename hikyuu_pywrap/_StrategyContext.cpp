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
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&,
                    const unordered_map<string, int>&>(),
           py::arg("stock_list"), py::arg("ktype_list"),
           py::arg("preload_num") = unordered_map<string, int>(),
           R"(__init__(self, stock_list, ktype_list, [preload_num={}])
        
  创建策略上下文

  :param stock_list: 需要加载的证券代码列表，如：["sz000001", "sz000002"], 如包含 'ALL', 表示加载全部
  :param ktype_list: 需要加载的K线类型列表, 如：["day", "min"], 未指定时取全局配置文件中配置的默认值
  :param preload_num: 预加载数量，默认为空，如：{"min_max": 100, "day_max": 200}. 未指定时取全局配置文件中配置的默认值
  :return: 策略上下文对象)")

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
      .def_property("preload_num", py::overload_cast<>(&StrategyContext::getPreloadNum, py::const_),
                    &StrategyContext::setPreloadNum, py::return_value_policy::copy, "预加载数量")

      .def("empty", &StrategyContext::empty, "上下文证券代码列表是否为空");
}
