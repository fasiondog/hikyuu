/*
 * _Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include "_Parameter.h"

using namespace boost::python;
using namespace hku;

bool (*parameter_eq)(const Parameter&, const Parameter&) = &operator==;
bool (*parameter_ne)(const Parameter&, const Parameter&) = &operator!=;
bool (*parameter_lt)(const Parameter&, const Parameter&) = &operator<;

void export_Parameter() {

    to_python_converter<boost::any, AnyToPython>();

    class_<Parameter>("Parameter")
            .def(self_ns::str(self))
            .def("__setitem__", &Parameter::set<object>)
            .def("__getitem__", &Parameter::get<boost::any>)
            .def("getNameList", &Parameter::getNameList)
            .def("have", &Parameter::have)
            .def("set", &Parameter::set<object>)
            .def("get", &Parameter::get<boost::any>)
            .def("getNameList", &Parameter::getNameList)
            .def("getValueList", &Parameter::getValueList)
            .def("getNameValueList", &Parameter::getNameValueList)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<Parameter>())
#endif
            ;

    def("parameter_eq", parameter_eq);
    def("parameter_ne", parameter_ne);
    def("parameter_lt", parameter_lt);
}

