/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <hikyuu/strategy/StrategyBase.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace hku;

class PyStrategyBase : public StrategyBase {
public:
    using StrategyBase::StrategyBase;

    void init() override {
        PYBIND11_OVERLOAD(void, StrategyBase, init);
    }

    void onTick() override {
        PYBIND11_OVERLOAD(void, StrategyBase, onTick);
    }

    void onBar(const KQuery::KType& ktype) override {
        PYBIND11_OVERLOAD(void, StrategyBase, onBar, ktype);
    }

    void onMarketOpen() override {
        PYBIND11_OVERLOAD(void, StrategyBase, onMarketOpen);
    }

    void onMarketClose() override {
        PYBIND11_OVERLOAD(void, StrategyBase, onMarketClose);
    }

    void onClock(TimeDelta detla) override {
        PYBIND11_OVERLOAD(void, StrategyBase, onClock, detla);
    }
};

void export_Strategy(py::module& m) {
    py::class_<StrategyBase, PyStrategyBase>(m, "StrategyBase")
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

      .def("run", &StrategyBase::run)
      .def("init", &StrategyBase::init)
      .def("on_tick", &StrategyBase::onTick)
      .def("on_bar", &StrategyBase::onBar)
      .def("on_market_open", &StrategyBase::onMarketOpen)
      .def("on_market_close", &StrategyBase::onMarketClose)
      .def("on_clock", &StrategyBase::onClock);
}