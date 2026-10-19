/*
 * _Environment.cpp
 *
 *  Created on: 2013-3-2
 *      Author: fasiondog
 */

/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/environment/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyEnvironmentBase : public EnvironmentBase {
    PY_CLONE(PyEnvironmentBase, EnvironmentBase)

public:
    PyEnvironmentBase() : EnvironmentBase() {
        m_is_python_object = true;
    }

    PyEnvironmentBase(const string& name) : EnvironmentBase(name) {
        m_is_python_object = true;
    }

    PyEnvironmentBase(const EnvironmentBase& base) : EnvironmentBase(base) {
        m_is_python_object = true;
    }

    bool _isValid(const Datetime& datetime, price_t value) const override {
        PYBIND11_OVERLOAD_NAME(bool, EnvironmentBase, "_is_valid", _isValid, datetime, value);
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, EnvironmentBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, EnvironmentBase, _reset, );
    }
};

void export_Environment(py::module& m) {
    py::class_<EnvironmentBase, EnvironmentPtr, PyEnvironmentBase>(
      m, "EnvironmentBase", py::dynamic_attr(),
      R"(The market environment strategy base class

A market environment is a series of dates with values; a date is valid when its stored value is
finite and greater than 0, which is exactly what is_valid tests.

The environment is designed to be shared across systems (the System parameter shared_ev defaults
to True): is_valid takes a shared lock on every call while setting the query and reset take the
exclusive lock, so all systems sharing one environment must use the same query, otherwise every
differing query reruns the full calculation under the exclusive lock.

The strategies can be combined with the operators &, |, +, -, * and /, and the result is itself
an EnvironmentBase that can be combined further (see the operator docstrings, e.g. __mul__).

The custom market environment strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _is_valid : [Optional] The subclass validity rule, by default "finite and > 0"
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const EnvironmentBase&>())
      .def(py::init<const string&>())

      .def("__str__", to_py_str<EnvironmentBase>)
      .def("__repr__", to_py_str<EnvironmentBase>)

      .def_property("name", py::overload_cast<>(&EnvironmentBase::name, py::const_),
                    py::overload_cast<const string&>(&EnvironmentBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &EnvironmentBase::getQuery, &EnvironmentBase::setQuery,
                    py::return_value_policy::copy,
                    R"(Set or get the query condition

    For a Python subclass the _calculate callback acquires the GIL while the mutex is held, so
    setting the query must not run concurrently with the other calls on the same object.)")

      .def("get_param", &EnvironmentBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (EnvironmentBase::*)(const std::string&, const boost::any&)>(
             &EnvironmentBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &EnvironmentBase::haveParam, "Whether the specified parameter exists")

      .def("is_valid", &EnvironmentBase::isValid, R"(is_valid(self, datetime)

    Whether the system is valid at the specified time

    :param Datetime datetime: the specified time
    :return: True valid | False invalid

    The validity is exactly "the stored value of the date is a finite value greater than 0"; a date
    that was never registered through _add_valid is invalid.)")

      .def("_is_valid", &EnvironmentBase::_isValid, py::arg("datetime"), py::arg("value"),
           R"(_is_valid(self, datetime, value)

    [Overload hook] The subclass validity rule, called by is_valid for every date registered
    through _add_valid

    :param Datetime datetime: the queried date
    :param float value: the stored value of the date
    :return: True valid | False invalid

    The base implementation treats a date as valid when its value is finite and greater than 0;
    override it to apply a custom rule based on the concrete value. The System judges the market
    validity solely through is_valid (one call per bar), so overriding this hook directly changes
    the trading behavior; note that the combination operators keep using the built-in "> 0" rule
    on their sides.)")

      .def("get_value", &EnvironmentBase::getValue, R"(get_value(self, datetime)

    Get the stored value of the specified date

    :param Datetime datetime: the specified time
    :return: the stored value; 0.0 for a date that was never registered

    0.0 is returned both for an invalid date and for an absent one; use is_valid to tell the
    validity.)")

      .def("get_values", &EnvironmentBase::getValues, R"(get_values(self)

    Get the values as a time-series indicator: one record per date registered through _add_valid,
    ordered by date

    :return: Indicator

    The date set and the meaning of the value depend on the operator that produced this object
    (e.g. Add/Sub keep the union of both sides including invalid dates, And/Or keep only the
    valid dates); only a finite value greater than 0 is valid.)")

      .def("_add_valid", &EnvironmentBase::_addValid, py::arg("datetime"), py::arg("value") = 1.0,
           R"(_add_valid(self, datetime[, value=1.0])

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time
    :param float value: defaulting to 1.0; greater than 0 means valid, and less than or equal to 0 means invalid

    A non-finite value (NaN/inf) is discarded, and adding the same date twice accumulates the
    value instead of replacing it.

    It takes no lock itself, so it can only be called from _calculate (which runs while the object
    is locked); calling it from elsewhere would race with is_valid.)")

      .def("reset", &EnvironmentBase::reset,
           R"(The reset operation

    For a Python subclass the _reset callback acquires the GIL while the mutex is held, so it must
    not run concurrently with the other calls on the same object.)")
      .def("clone", &EnvironmentBase::clone,
           R"(The clone operation

    The subclass _clone callback is invoked without holding the mutex.)")
      .def("_reset", &EnvironmentBase::_reset,
           "[Overload interface] The subclass reset interface, used to reset the internal private "
           "variables")
      .def("_calculate", &EnvironmentBase::_calculate,
           "[Overload interface] The subclass calculation interface")

      .def(
        "__and__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self & other; },
        R"(__and__(self, other)

    And (self & other): the day is valid only where both sides are valid (> 0), with the value
    fixed to 1.0. Both sides must be present, otherwise the result is always invalid.)")

      .def(
        "__or__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self | other; },
        R"(__or__(self, other)

    Or (self | other): the day is valid where at least one side is valid (> 0), with the value
    fixed to 1.0. When one side is empty, the single-side mode is used.)")

      .def(
        "__add__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self + other; },
        R"(__add__(self, other)

    Add (self + other): every date of both sides is kept, valid or not; the value is the sum, and
    a date present on only one side takes that side's value. When one side is empty, the
    single-side mode is used.)")

      .def(
        "__sub__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self - other; },
        R"(__sub__(self, other)

    Sub (self - other): every date of both sides is kept, valid or not; the value is the
    difference, and a date present only on the right side takes -other. When one side is empty,
    the single-side mode is used.)")

      .def(
        "__mul__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self * other; },
        R"(__mul__(self, other)

    Multi (self * other): only the dates where both sides agree are kept, and the signs of the two
    sides decide the result:

        left > 0 and right > 0 : self * other (valid)
        left < 0 and right < 0 : -(abs(self) * abs(other)) (invalid)
        mixed sign             : not output (disagreement, no combined strength)
        either side is 0       : not output (that side has no value)

    Both sides must be present, otherwise the result is always invalid.)")

      .def(
        "__truediv__",
        [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self / other; },
        R"(__truediv__(self, other)

    Div (self / other): only the dates where both sides agree are kept, and the signs of the two
    sides decide the result:

        left > 0 and right > 0 : self / other (valid)
        left < 0 and right < 0 : -(abs(self) / abs(other)) (invalid)
        mixed sign             : not output (disagreement, no combined strength)
        either side is 0       : not output (that side has no value)

    Both sides must be present, otherwise the result is always invalid.)")

        DEF_PICKLE(EnvironmentPtr);

    m.def("EV_TwoLine", EV_TwoLine, py::arg("fast"), py::arg("slow"), py::arg("market") = "SH",
          R"(EV_TwoLine(fast, slow[, market = 'SH'])

    The fast/slow line strategy; when the fast line of the market index is greater than the slow line, the market is valid, otherwise invalid.

    :param Indicator fast: the fast line indicator
    :param Indicator slow: the slow line indicator
    :param string market: the market name)");

    m.def("EV_Bool", EV_Bool, py::arg("ind"), py::arg("market") = "SH",
          R"(EV_Bool(ind, market='SH')

    The boolean signal generator market environment

    :param Indicator ind: a bool-type indicator; if the corresponding position in the indicator is >0, it means the market is valid, otherwise invalid
    :param str market: the specified market, used to get the corresponding trading calendar)");

    m.def("EV_Manual", EV_Manual,
          R"(EV_Manual()

    A manual environment whose valid dates can only be registered through _add_valid, used for
    the testing or other purposes.

    The registered dates live until the query changes, so set the query first and register the
    dates afterwards. Note that the combination operators re-run each side's _calculate through
    setQuery, which clears a manual side; a combined result can be registered directly instead.)");
}
