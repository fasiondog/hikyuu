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

class PyStrategyBase : public StrategyBase {
public:
    using StrategyBase::StrategyBase;

    void init() override {
        PYBIND11_OVERLOAD(void, StrategyBase, init);
    }

    void onChange(const Stock& stk, const SpotRecord& spot) override {
        PYBIND11_OVERLOAD(void, StrategyBase, onChange, stk, spot);
    }

    virtual void onReceivedSpot(Datetime revTime) override {
        PYBIND11_OVERLOAD(void, StrategyBase, onReceivedSpot, revTime);
    }
};

void export_Strategy(py::module& m) {
    py::class_<StrategyBase, StrategyPtr, PyStrategyBase>(m, "StrategyBase")
      .def(py::init<>())
      .def_property("name", py::overload_cast<>(&StrategyBase::name, py::const_),
                    py::overload_cast<const string&>(&StrategyBase::name),
                    py::return_value_policy::copy, "策略名称")

      .def_property_readonly("sm", &StrategyBase::getSM, py::return_value_policy::reference,
                             "获取 StockManager 实例")
      .def_property("tm", &StrategyBase::getTM, &StrategyBase::setTM, "账户管理")
      .def_property("start_datetime", py::overload_cast<>(&StrategyBase::startDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&StrategyBase::startDatetime), "起始日期")
      .def_property("stock_list", py::overload_cast<>(&StrategyBase::getStockCodeList, py::const_),
                    py::overload_cast<const vector<string>&>(&StrategyBase::setStockCodeList),
                    py::return_value_policy::copy, "股票代码列表")
      .def_property("ktype_list", py::overload_cast<>(&StrategyBase::getKTypeList, py::const_),
                    py::overload_cast<const vector<KQuery::KType>&>(&StrategyBase::setKTypeList),
                    py::return_value_policy::copy, "需要的K线类型")

      .def("init", &StrategyBase::init)
      .def("on_change", &StrategyBase::onChange)
      .def("on_received_spot", &StrategyBase::onReceivedSpot)
      .def("start", &PyStrategyBase::start)
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
        py::arg("func"), py::arg("time"), py::arg("ignore_holiday") = true)
      //   .def("run_daily", &StrategyBase::runDaily)
      //   .def("run_daily_at", &StrategyBase::runDailyAt)
      ;
}