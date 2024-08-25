/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <hikyuu/strategy/Strategy.h>
#include <hikyuu/strategy/BrokerTradeManager.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace hku;

void export_Strategy(py::module& m) {
    py::class_<Strategy, StrategyPtr>(m, "Strategy")
      .def(py::init<>())
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&, const std::string&,
                    const std::string&>(),
           py::arg("code_list"), py::arg("ktype_list"), py::arg("name") = "Strategy",
           py::arg("config") = "")
      .def_property("name", py::overload_cast<>(&Strategy::name, py::const_),
                    py::overload_cast<const string&>(&Strategy::name),
                    py::return_value_policy::copy, "策略名称")

      .def_property_readonly("context", &Strategy::context, py::return_value_policy::copy,
                             "策略上下文")

      .def("start", &Strategy::start)
      .def("on_change",
           [](Strategy& self, py::object func) {
               HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
               py::object c_func = func.attr("__call__");
               auto new_func = [=](const Stock& stk, const SpotRecord& spot) {
                   try {
                       c_func(stk, spot);
                   } catch (const std::exception& e) {
                       HKU_ERROR(e.what());
                   } catch (...) {
                       HKU_ERROR("Unknown error!");
                   }
               };
               self.onChange(new_func);
           })
      .def("on_received_spot",
           [](Strategy& self, py::object func) {
               HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
               py::object c_func = func.attr("__call__");
               auto new_func = [=](Datetime revTime) {
                   try {
                       c_func(revTime);
                   } catch (const std::exception& e) {
                       HKU_ERROR(e.what());
                   } catch (...) {
                       HKU_ERROR("Unknown error!");
                   }
               };
               self.onReceivedSpot(new_func);
           })
      .def(
        "run_daily",
        [](Strategy& self, py::object func, const TimeDelta& time, std::string market,
           bool ignore_market) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            py::object c_func = func.attr("__call__");
            auto new_func = [=]() {
                try {
                    c_func();
                } catch (const std::exception& e) {
                    HKU_ERROR(e.what());
                } catch (...) {
                    HKU_ERROR("Unknown error!");
                }
            };
            self.runDaily(new_func, time, market, ignore_market);
        },
        py::arg("func"), py::arg("time"), py::arg("market") = "SH",
        py::arg("ignore_market") = false)
      .def(
        "run_daily_at",
        [](Strategy& self, py::object func, const TimeDelta& time, bool ignore_holiday) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            py::object c_func = func.attr("__call__");
            auto new_func = [=]() {
                try {
                    c_func();
                } catch (const std::exception& e) {
                    HKU_ERROR(e.what());
                } catch (...) {
                    HKU_ERROR("Unknown error!");
                }
            };
            self.runDailyAt(new_func, time, ignore_holiday);
        },
        py::arg("func"), py::arg("time"), py::arg("ignore_holiday") = true);

    m.def("crtBrokerTM", crtBrokerTM, py::arg("broker"), py::arg("cost_func") = TC_Zero(),
          py::arg("name") = "SYS");

    m.def("run_in_strategy",
          py::overload_cast<const SYSPtr&, const Stock&, const KQuery&, const OrderBrokerPtr&,
                            const TradeCostPtr&>(runInStrategy),
          py::arg("sys"), py::arg("stock"), py::arg("query"), py::arg("broker"),
          py::arg("cost_func"));
    m.def("run_in_strategy",
          py::overload_cast<const PFPtr&, const KQuery&, int, const OrderBrokerPtr&,
                            const TradeCostPtr&>(runInStrategy),
          py::arg("pf"), py::arg("query"), py::arg("adjust_cycle"), py::arg("broker"),
          py::arg("cost_func"));
}