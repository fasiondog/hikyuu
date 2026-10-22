/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/stoploss/build_in.h>
#include "../gil_policy.h"
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
        PYBIND11_OVERRIDE(void, StoplossBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERRIDE(void, StoplossBase, _reset, );
    }

    price_t _getPrice(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERRIDE_PURE_NAME(price_t, StoplossBase, "_get_price", _getPrice, datetime,
                                    price);
    }

    price_t _getShortPrice(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERRIDE_NAME(price_t, StoplossBase, "_get_short_price", _getShortPrice, datetime,
                               price);
    }
};

void export_Stoploss(py::module& m) {
    py::class_<StoplossBase, StoplossPtr, PyStoplossBase>(
      m, "StoplossBase", py::dynamic_attr(),
      R"(The stop-loss/take-profit algorithm base class
The custom stop-loss/take-profit strategy interfaces:

    - _get_price : [Required] Compute the raw long stop-loss price
    - _get_short_price : [Required] Compute the raw short stop-loss price (default returns 0; a short stop-loss must be above the planned trade price)
    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables

How often the hooks run: used as a stop-loss they run on the entry path only (about once or twice per trade, the level is then fixed in the position record); used as a take-profit they run once per bar while the position is held, since the take-profit level is trailing.

Return value contract of the hooks:
    - > 0 : a valid stop-loss level, expressed in the same (adjusted) price coordinate as the KData passed to setTO; the system maps it back to the original price
    - 0 : no stop-loss on this bar; the trade still takes place without stop-loss protection
    - negative / NaN / inf : invalid; the level is reported as Null and the system opens the trade without stop-loss (sized with the full planned price as risk), the entry is not skipped
The system still skips a long buy when the level is not lower than the planned buy price, and a short sell when the level is not above the planned trade price. When the instance is used as a take-profit, 0 or an invalid level only means no take-profit on this bar, the trade is not affected.)")
      .def(py::init<>())
      .def(py::init<const StoplossBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<StoplossBase>)
      .def("__repr__", to_py_str<StoplossBase>)

      .def_property("name", py::overload_cast<>(&StoplossBase::name, py::const_),
                    py::overload_cast<const string&>(&StoplossBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("tm", &StoplossBase::getTM, &StoplossBase::setTM,
                    "The associated trade manager instance")
      // The GIL is always released so other threads can run during the (possibly slow)
      // calculation; the trampolines of a Python subclass re-acquire the GIL per callback
      .def_property(
        "to", &StoplossBase::getTO,
        [](StoplossBase& self, const KData& kdata) {
            ScopeGilRelease gil(!self.hasPythonPart());
            self.setTO(kdata);
        },
        "The associated trading object")

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

    [Public method] Get the planned stop-loss price of this expected trade (a buy); return 0 if none. A negative or non-finite result of the hook is invalid: Null is returned and the system opens the trade without stop-loss (sized with the full planned price as risk). The system still skips the trade when the level is not lower than the planned buy price. It calls the subclass hook _get_price. Do not override this method; override _get_price instead.

    :param Datetime datetime: the trading time
    :param float price: the planned buy price
    :return: the stop-loss price, or Null if the hook returned an invalid level
    :rtype: float)")

      .def("get_short_price", &StoplossBase::getShortPrice,
           R"(get_short_price(self, datetime, price)

    [Public method] Get the planned stop-loss price of this expected trade (a short sell); return 0 if none. A negative or non-finite result of the hook is invalid: Null is returned and the system opens the short position without stop-loss (sized with the full planned price as risk). The system still skips the trade when the level is not above the planned trade price, since the cover would trigger immediately. It calls the subclass hook _get_short_price. Do not override this method; override _get_short_price instead.

    :param Datetime datetime: the trading time
    :param float price: the planned trade price
    :return: the short stop-loss price, or Null if the hook returned an invalid level
    :rtype: float)")

      .def("_get_price", &StoplossBase::_getPrice,
           R"(_get_price(self, datetime, price)

    [Override hook] Compute the raw long stop-loss price; the public get_price delegates to this hook. Used as a stop-loss it is called on the entry path only; used as a take-profit it is called once per bar while the position is held. Return the planned stop-loss price (0 means no stop-loss) in the same (adjusted) price coordinate as the KData passed to setTO. A negative or non-finite result is invalid, and the system opens the trade without stop-loss.

    .. note::
        In general, stop-loss and take-profit algorithms are interchangeable. The difference is that a stop-loss implementation may use the planned trade price—for example, setting the stop 30% below the buy price—whereas a take-profit ignores the price argument and treats it as 0.0. In practice, even stop-loss implementations are better off ignoring price: a stop based on, say, a level relative to the previous day's low does not need it.

    :param Datetime datetime: the trading time
    :param float price: the planned buy price
    :return: the stop-loss price
    :rtype: float)")

      .def("_get_short_price", &StoplossBase::_getShortPrice,
           R"(_get_short_price(self, datetime, price)

    [Override hook] Compute the raw short stop-loss price; the public get_short_price delegates to this hook. Used as a short stop-loss it is called on the entry path only; used as a short take-profit it is called once per bar while the position is held. Return the level in the same (adjusted) price coordinate as the KData passed to setTO. Default returns 0 (no short stop-loss); a negative or non-finite result is invalid, and the system opens the short position without stop-loss. Unlike a long stop-loss, a short one must be above the planned trade price (otherwise the system skips the short entry), and must not fall back to the long-side _get_price result.

    :param Datetime datetime: the trading time
    :param float price: the planned trade price
    :return: the short stop-loss price
    :rtype: float)")

      .def("reset",
           [](StoplossBase& self) {
               ScopeGilRelease gil(!self.hasPythonPart());
               self.reset();
           },
           "The reset operation")
      .def("clone",
           [](StoplossBase& self) -> StoplossPtr {
               ScopeGilRelease gil(!self.hasPythonPart());
               return self.clone();
           },
           "The clone operation")
      .def("_calculate", &StoplossBase::_calculate,
           "[Overload interface] The subclass calculation interface. Do not call "
           "super()._calculate() in a Python override (causes infinite recursion).")
      .def("_reset", &StoplossBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the internal private "
           "variables")

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
    :param double p: the noise coefficient, defaulting to 2; it must be finite and greater than 0, otherwise the creation raises an exception
    :return: the stop-loss/take-profit strategy instance)");
}