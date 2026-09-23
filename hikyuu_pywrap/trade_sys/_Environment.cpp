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
    :return: True valid | False invalid)")

      .def("_add_valid", &EnvironmentBase::_addValid, py::arg("datetime"), py::arg("value") = 1.0,
           R"(_add_valid(self, datetime)

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time
    :param float value: defaulting to 1.0; greater than 0 means valid, and less than or equal to 0 means invalid)")

      .def("reset", &EnvironmentBase::reset, "The reset operation")
      .def("clone", &EnvironmentBase::clone, "The clone operation")
      .def("_reset", &EnvironmentBase::_reset,
           "[Overload interface] The subclass reset interface, used to reset the internal private "
           "variables")
      .def("_calculate", &EnvironmentBase::_calculate,
           "[Overload interface] The subclass calculation interface")

      .def("__and__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self & other; })

      .def("__or__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self | other; })

      .def("__add__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self + other; })

      .def("__sub__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self - other; })

      .def("__mul__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self * other; })

      .def("__truediv__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) { return self / other; })

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