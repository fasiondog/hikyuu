/*
 * _Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <hikyuu/utilities/Parameter.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

bool (*parameter_eq)(const Parameter&, const Parameter&) = &operator==;
bool (*parameter_ne)(const Parameter&, const Parameter&) = &operator!=;
bool (*parameter_lt)(const Parameter&, const Parameter&) = &operator<;

static std::string Parameter_to_str(const Parameter& param) {
    std::stringstream ss;
    ss << param;
    return ss.str();
}

void export_Parameter(py::module& m) {
    py::class_<Parameter>(m, "Parameter", "参数类，供需要命名参数设定的类使用，类似于 dict")
      .def("__str__", Parameter_to_str)
      .def("__repr__", Parameter_to_str)

      .def("__contains__", &Parameter::have)
      .def("__setitem__", &Parameter::set<boost::any>)
      .def("__getitem__", &Parameter::get<boost::any>)
      .def("have", &Parameter::have, "Return True if there is a parameter for the specified name.")
      .def("set", &Parameter::set<boost::any>)
      .def("get", &Parameter::get<boost::any>)
      .def("type", &Parameter::type,
           "Get the type name of the specified parameter, return 'string' | 'int' | 'double' | "
           "'bool' | 'Stock' | 'KQuery' | 'KData' | 'PriceList' | 'DatetimeList'")
      .def("get_name_list", &Parameter::getNameList, "Get all the parameter names list")
      .def("get_name_value_list", &Parameter::getNameValueList,
           "Return a string, like 'name1=val1,name2=val2,...'")

      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self < py::self)

        DEF_PICKLE(Parameter);
}
