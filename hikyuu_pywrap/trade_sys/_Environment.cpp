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
greater than 0, which is exactly what is_valid tests.

The strategies can be combined with the operators &, |, +, -, * and /, and the result is itself
an EnvironmentBase that can be combined further (see the operator docstrings, e.g. __mul__).

The custom market environment strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
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
                    py::return_value_policy::copy, "Set or get the query condition")

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

    The validity is exactly "the stored value of the date is greater than 0"; a date that was
    never registered through _add_valid is invalid.)")

      .def("_add_valid", &EnvironmentBase::_addValid, py::arg("datetime"), py::arg("value") = 1.0,
           R"(_add_valid(self, datetime[, value=1.0])

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time
    :param float value: defaulting to 1.0; greater than 0 means valid, and less than or equal to 0 means invalid

    A non-finite value (NaN/inf) is discarded, and adding the same date twice accumulates the
    value instead of replacing it.)")

      .def("reset", &EnvironmentBase::reset, "The reset operation")
      .def("clone", &EnvironmentBase::clone, "The clone operation")
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
}
