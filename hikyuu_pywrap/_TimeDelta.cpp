/*
 * _TimeDelta.cpp
 *
 *  Copyright (C) 2019 hikyuu.org
 *
 *  Created on: 2019-12-14
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TimeDelta_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

double (TimeDelta::*TimeDelta_div_1)(TimeDelta) const = &TimeDelta::operator/;
TimeDelta (TimeDelta::*TimeDelta_div_2)(double) const = &TimeDelta::operator/;

void export_TimeDelta() {
    class_<TimeDelta>("TimeDelta", init<>())
      .def(init<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(
        (arg("days") = 0, arg("hours") = 0, arg("minutes") = 0, arg("seconds") = 0, arg("milliseconds") = 0,
         arg("microseconds") = 0)))
      //.def(self_ns::str(self))
      .def("__str__", &TimeDelta::str)
      .def("__repr__", &TimeDelta::repr)
      .add_property("days", &TimeDelta::days)
      .add_property("hours", &TimeDelta::hours)
      .add_property("minutes", &TimeDelta::minutes)
      .add_property("seconds", &TimeDelta::seconds)
      .add_property("milliseconds", &TimeDelta::milliseconds)
      .add_property("microseconds", &TimeDelta::microseconds)
      .add_property("ticks", &TimeDelta::ticks)
      .def("isNegative", &TimeDelta::isNegative)
      .def("max", &TimeDelta::max)
      .staticmethod("max")
      .def("min", &TimeDelta::min)
      .staticmethod("min")
      .def("resolution", &TimeDelta::resolution)
      .staticmethod("resolution")
      .def("maxTicks", &TimeDelta::maxTicks)
      .staticmethod("maxTicks")
      .def("minTicks", &TimeDelta::minTicks)
      .staticmethod("minTicks")
      .def("fromTicks", &TimeDelta::fromTicks)
      .staticmethod("fromTicks")

      .def("__eq__", &TimeDelta::operator==)
      .def("__ne__", &TimeDelta::operator!=)
      .def("__gt__", &TimeDelta::operator>)
      .def("__lt__", &TimeDelta::operator<)
      .def("__ge__", &TimeDelta::operator>=)
      .def("__le__", &TimeDelta::operator>=)

      .def("__abs__", &TimeDelta::abs)
      .def("__add__", &TimeDelta::operator+)
      .def("__sub__", &TimeDelta::operator-)
      .def("__mul__", &TimeDelta::operator*)
      .def("__rmul__", &TimeDelta::operator*)
      .def("__truediv__", TimeDelta_div_1)
      .def("__truediv__", TimeDelta_div_2)
      .def("__mod__", &TimeDelta::operator%)

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TimeDelta>())
#endif
      ;

    def("Days", Days);
    def("Hours", Hours);
    def("Minutes", Minutes);
    def("Seconds", Seconds);
    def("Milliseconds", Milliseconds);
    def("Microseconds", Microseconds);
}
