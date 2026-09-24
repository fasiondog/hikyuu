/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/stoploss/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyStoplossBase : public StoplossBase {
    PY_CLONE(PyStoplossBase, StoplossBase)

public:
    PyStoplossBase() : StoplossBase() {
        m_is_python_object = true;
    }

    PyStoplossBase(const string& name) : StoplossBase(name) {
        m_is_python_object = true;
    }

    PyStoplossBase(const StoplossBase& base) : StoplossBase(base) {
        m_is_python_object = true;
    }

    void _calculate() override {
        PYBIND11_OVERLOAD(void, StoplossBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, StoplossBase, _reset, );
    }

    price_t getPrice(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, StoplossBase, "get_price", getPrice, datetime, price);
    }

    price_t getShortPrice(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERLOAD_NAME(price_t, StoplossBase, "get_short_price", getShortPrice, datetime,
                               price);
    }
};

void export_Stoploss(py::module& m) {
    py::class_<StoplossBase, StoplossPtr, PyStoplossBase>(m, "StoplossBase", py::dynamic_attr(),
                                                          R"(The stop-loss/take-profit algorithm base class
The custom stop-loss/take-profit strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const StoplossBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<StoplossBase>)
      .def("__repr__", to_py_str<StoplossBase>)

      .def_property("name", py::overload_cast<>(&StoplossBase::name, py::const_),
                    py::overload_cast<const string&>(&StoplossBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("tm", &StoplossBase::getTM, &StoplossBase::setTM, "The associated trade manager instance")
      .def_property("to", &StoplossBase::getTO, &StoplossBase::setTO, "The associated trading object")

      .def("get_param", &StoplossBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (StoplossBase::*)(const std::string&, const boost::any&)>(
             &StoplossBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &StoplossBase::haveParam, "Whether the specified parameter exists")

      .def("get_price", &StoplossBase::getPrice, R"(get_price(self, datetime, price)

    [Overload interface] Get the planned stop-loss price of this expected trade (buy); if there is no stop-loss price, return 0. It is used by the system to query the planned stop-loss price of this trade from the stop-loss strategy module before executing the trade.

    .. note::
        Generally, the stop-loss and the take-profit algorithms can be interchanged, but the getPrice of the stop-loss can be passed the planned trading price, e.g. using 30% of the buy price as the stop-loss. The take-profit does not consider the passed price parameter, i.e. it considers the price to be 0.0. In fact, even for the stop-loss, it is not recommended to use the price parameter; e.g. if 30% of the previous day's lowest price can be used as the stop-loss, the price parameter does not need to be considered.

    :param Datetime datetime: the trading time
    :param float price: the planned buy price
    :return: the stop-loss price
    :rtype: float)")

      .def("get_short_price", &StoplossBase::getShortPrice)

      .def("reset", &StoplossBase::reset, "The reset operation")
      .def("clone", &StoplossBase::clone, "The clone operation")
      .def("_calculate", &StoplossBase::_calculate, "[Overload interface] The subclass calculation interface")
      .def("_reset", &StoplossBase::_reset, "[Overload interface] The subclass reset interface, resetting the internal private variables")

        DEF_PICKLE(StoplossPtr);

    m.def("ST_FixedPercent", ST_FixedPercent, py::arg("p") = 0.03, R"(ST_FixedPercent([p=0.03])

    The fixed percentage stop-loss strategy, i.e. stopping the loss when the price is below a certain percentage of the buy price

    :param float p: the percentage(0,1]
    :return: the stop-loss/take-profit strategy instance)");

    m.def("ST_Indicator", ST_Indicator, py::arg("ind"),
          R"(ST_Indicator(ind)

    Use a technical indicator as the stop-loss price. E.g. using the 10-day EMA as the stop-loss::

        ST_Indicator(EMA(CLOSE(), n=10))

    :param Indicator ind:
    :return: the stop-loss/take-profit strategy instance)");

    m.def("ST_Saftyloss", ST_Saftyloss, py::arg("n1") = 10, py::arg("n2") = 3, py::arg("p") = 2.0,
          R"(ST_Saftyloss([n1=10, n2=3, p=2.0])

    See Come Into My Trading Room (2007, 地震出版社) by Alexander Elder, P202
    The calculation description: within the lookback period (generally 10 to 20 days), sum all the lengths of the downward penetrations and divide by the number of the downward penetrations,
    to get the average noise (i.e. the total length of all the lowest prices below the lowest price of the previous day within the lookback period divided by the number of the times), and subtract
    (the average noise of the previous day multiplied by a multiple) from the lowest price of today to get the stop line. To offset the fluctuation and ensure the upward movement of the stop line,
    take the highest value within the N days (generally 3 days) on the basis of the above result

    :param int n1: the lookback time window for calculating the average noise, defaulting to 10 days
    :param int n2: take the highest value within the n2 days for the preliminary stop line, defaulting to 3
    :param double p: the noise coefficient, defaulting to 2
    :return: the stop-loss/take-profit strategy instance)");
}