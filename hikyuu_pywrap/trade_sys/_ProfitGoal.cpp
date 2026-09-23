/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/profitgoal/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyProfitGoalBase : public ProfitGoalBase {
    PY_CLONE(PyProfitGoalBase, ProfitGoalBase)

public:
    PyProfitGoalBase() : ProfitGoalBase() {
        m_is_python_object = true;
    }

    PyProfitGoalBase(const string& name) : ProfitGoalBase(name) {
        m_is_python_object = true;
    }

    PyProfitGoalBase(const ProfitGoalBase& base) : ProfitGoalBase(base) {
        m_is_python_object = true;
    }

    void buyNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "buy_notify", buyNotify, tr);
    }

    void sellNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "sell_notify", sellNotify, tr);
    }

    price_t getGoal(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, ProfitGoalBase, "get_goal", getGoal, datetime, price);
    }

    price_t getShortGoal(const Datetime& date, price_t price) override {
        PYBIND11_OVERLOAD_NAME(price_t, ProfitGoalBase, "get_short_goal", getShortGoal, date,
                               price);
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, ProfitGoalBase, _reset, );
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "_calculate", _calculate, );
    };
};

void export_ProfitGoal(py::module& m) {
    py::class_<ProfitGoalBase, PGPtr, PyProfitGoalBase>(m, "ProfitGoalBase", py::dynamic_attr(),
                                                        R"(The profit goal strategy base class
    
The custom profit goal strategy interfaces:

- getGoal : [Required] Get the target price
- _calculate : [Required] The subclass calculation interface
- _clone : [Required] The clone interface
- _reset : [Optional] Reload the private variables
- buyNotify : [Optional] Receive the actual buy notification, reserved for the multiple position increase/decrease processing
- sellNotify : [Optional] Receive the actual sell notification, reserved for the multiple position increase/decrease processing)")

      .def(py::init<>())
      .def(py::init<const ProfitGoalBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<ProfitGoalBase>)
      .def("__repr__", to_py_str<ProfitGoalBase>)

      .def_property("name", py::overload_cast<>(&ProfitGoalBase::name, py::const_),
                    py::overload_cast<const string&>(&ProfitGoalBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &ProfitGoalBase::getTO, &ProfitGoalBase::setTO, "Set or get the trading object")
      .def_property("tm", &ProfitGoalBase::getTM, &ProfitGoalBase::setTM, "Set or get the trade manager account")

      .def("get_param", &ProfitGoalBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (ProfitGoalBase::*)(const std::string&, const boost::any&)>(
             &ProfitGoalBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ProfitGoalBase::haveParam, "Whether the specified parameter exists")

      .def("buy_notify", &ProfitGoalBase::buyNotify,
           R"(buy_notify(self, trade_record)
    
    [Overload interface] When the trading system performs the actual buy operation, notify the trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual buy trade record when the actual buying occurs)")

      .def("sell_notify", &ProfitGoalBase::sellNotify,
           R"(sell_notify(self, trade_record)
    
    [Overload interface] When the trading system performs the actual sell operation, notify the actual trade changes; it only needs to be overloaded when there are multiple position increases/decreases
        
    :param TradeRecord trade_record: the actual sell trade record when the actual selling occurs)")

      .def("get_goal", &ProfitGoalBase::getGoal, R"(get_goal(self, datetime, price)

    [Overload interface] Get the profit goal price; returning constant.null_price means the goal is not limited; returning 0 means it needs to be sold

    :param Datetime datetime: the current time
    :param float price: the current price
    :return: the target price
    :rtype: float)")

      //.def("getShortGoal", &ProfitGoalBase::getShortGoal, &ProfitGoalWrap::default_getShortGoal)

      .def("reset", &ProfitGoalBase::reset, "The reset operation")
      .def("clone", &ProfitGoalBase::clone, "The clone operation")
      .def("_calculate", &ProfitGoalBase::_calculate, "[Overload interface] The subclass calculation interface")
      .def("_reset", &ProfitGoalBase::_reset, "[Overload interface] The subclass reset interface, resetting the internal private variables")

        DEF_PICKLE(PGPtr);

    m.def("PG_NoGoal", PG_NoGoal, R"(PG_NoGoal()

    The no profit goal strategy, usually for testing or comparison.
    
    :return: the profit goal strategy instance)");

    m.def("PG_FixedPercent", PG_FixedPercent, py::arg("p") = 0.2, R"(PG_FixedPercent([p = 0.2])

    The fixed percentage profit goal; the target price = the buy price * (1 + p)
    
    :param float p: the percentage
    :return: the profit goal strategy instance)");

    m.def("PG_FixedHoldDays", PG_FixedHoldDays, py::arg("days") = 5, R"(PG_FixedHoldDays([days=5])

    The fixed holding days profit goal strategy
    
    :param int days: the allowed holding days (counted by the trading days), defaulting to 5 days
    :return: the profit goal strategy instance)");
}