/*
 * _Condition.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/condition/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyConditionBase : public ConditionBase {
    PY_CLONE(PyConditionBase, ConditionBase)

public:
    PyConditionBase() : ConditionBase() {
        m_is_python_object = true;
    }

    PyConditionBase(const string& name) : ConditionBase(name) {
        m_is_python_object = true;
    }

    PyConditionBase(const ConditionBase& base) : ConditionBase(base) {
        m_is_python_object = true;
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, ConditionBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, ConditionBase, _reset, );
    }
};

void export_Condition(py::module& m) {
    py::class_<ConditionBase, ConditionPtr, PyConditionBase>(
      m, "ConditionBase", py::dynamic_attr(),
      R"(The system valid condition base class; the custom system valid condition interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const ConditionBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<ConditionBase>)
      .def("__repr__", to_py_str<ConditionBase>)

      .def_property("name", py::overload_cast<>(&ConditionBase::name, py::const_),
                    py::overload_cast<const string&>(&ConditionBase::name),
                    py::return_value_policy::copy, "Name")

      .def_property("to", &ConditionBase::getTO, &ConditionBase::setTO, "Set or get the trading object")
      .def_property("tm", &ConditionBase::getTM, &ConditionBase::setTM, "Set or get the trade manager account")
      .def_property("sg", &ConditionBase::getSG, &ConditionBase::setSG, "Set or get the trading signal generator")

      .def("get_param", &ConditionBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (ConditionBase::*)(const std::string&, const boost::any&)>(
             &ConditionBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ConditionBase::haveParam, "Whether the specified parameter exists")

      .def("is_valid", &ConditionBase::isValid, R"(is_valid(self, datetime)

    Whether the system is valid at the specified time

    :param Datetime datetime: the specified time
    :return: True valid | False invalid)")

      .def("reset", &ConditionBase::reset, "The reset operation")
      .def("clone", &ConditionBase::clone, "The clone operation")

      .def("get_datetime_list", &ConditionBase::getDatetimeList, R"(get_datetime_list(self)
    
    Get the dates when the system is valid. Note that only the list of the dates when the system is valid is returned, which is not the same length as the trading object)")

      .def("get_values", &ConditionBase::getValues, R"(get_values(self)
           
    Get the actual values in the form of an indicator, with the same length as the trading object; 0 means invalid, and 1 means the system is valid)")

      .def("_add_valid", &ConditionBase::_addValid, py::arg("datetime"), py::arg("value") = 1.0,
           R"(_add_valid(self, datetime)

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time)")

      .def("_calculate", &ConditionBase::_calculate, "[Overload interface] The subclass calculation interface")
      .def("_reset", &ConditionBase::_reset, "[Overload interface] The subclass reset interface, resetting the internal private variables")

      .def("__len__", &ConditionBase::size)

      .def("__getitem__",
           [](const ConditionPtr& self, int64_t i) {
               size_t total = self->size();
               int64_t pos = i < 0 ? total + i : i;
               return self->at(pos);
           })

      .def("__and__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self & other; })

      .def("__or__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self | other; })

      .def("__add__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self + other; })

      .def("__sub__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self - other; })

      .def("__mul__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self * other; })

      .def("__truediv__",
           [](const ConditionPtr& self, const ConditionPtr& other) { return self / other; })

        DEF_PICKLE(ConditionPtr);

    m.def("CN_OPLine", CN_OPLine, R"(CN_OPLine(ind)

    Fixedly use the minimum trading quantity of the stock to trade; calculate the ind value of the equity curve; when the equity curve is higher than ind, the system is valid, otherwise invalid.

    :param Indicator ind: the Indicator instance
    :return: the system valid condition instance
    :rtype: ConditionBase)");

    m.def("CN_Bool", CN_Bool, R"(CN_Bool(ind)

    The boolean signal generator system valid condition; if the corresponding position in the indicator is >0, it means the system is valid, otherwise invalid

    :param Indicator ind: a bool-type indicator, with the KData as the input
    :return: the system valid condition instance
    :rtype: ConditionBase)");
}