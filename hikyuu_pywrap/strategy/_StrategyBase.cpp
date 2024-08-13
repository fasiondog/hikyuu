/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <hikyuu/strategy/StrategyBase.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace hku;

void export_Strategy(py::module& m) {
    py::class_<StrategyBase, StrategyPtr>(m, "StrategyBase")
      .def(py::init<>())
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&, const std::string&,
                    const std::string&>(),
           py::arg("code_list"), py::arg("ktype_list"), py::arg("name") = "Strategy",
           py::arg("config") = "")
      .def_property("name", py::overload_cast<>(&StrategyBase::name, py::const_),
                    py::overload_cast<const string&>(&StrategyBase::name),
                    py::return_value_policy::copy, "策略名称")

      .def_property("start_datetime", py::overload_cast<>(&StrategyBase::startDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&StrategyBase::startDatetime), "起始日期")
      .def_property("stock_list", py::overload_cast<>(&StrategyBase::getStockCodeList, py::const_),
                    py::overload_cast<const vector<string>&>(&StrategyBase::setStockCodeList),
                    py::return_value_policy::copy, "股票代码列表")
      .def_property("ktype_list", py::overload_cast<>(&StrategyBase::getKTypeList, py::const_),
                    py::overload_cast<const vector<KQuery::KType>&>(&StrategyBase::setKTypeList),
                    py::return_value_policy::copy, "需要的K线类型")

      .def("start", &StrategyBase::start)
      .def("on_change",
           [](StrategyBase& self, py::object func) {
               HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
               py::object c_func = func.attr("__call__");
               self.onChange(c_func);
           })
      .def("on_received_spot",
           [](StrategyBase& self, py::object func) {
               HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
               py::object c_func = func.attr("__call__");
               self.onReceivedSpot(c_func);
           })
      .def("run_daily",
           [](StrategyBase& self, py::object func, const TimeDelta& time) {
               HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
               py::object c_func = func.attr("__call__");
               self.runDaily(c_func, time);
           })
      .def(
        "run_daily_at",
        [](StrategyBase& self, py::object func, const TimeDelta& time, bool ignore_holiday) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            py::object c_func = func.attr("__call__");
            self.runDailyAt(c_func, time, ignore_holiday);
        },
        py::arg("func"), py::arg("time"), py::arg("ignore_holiday") = true);
}