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

void export_TimeDelta() {
    class_<TimeDelta>("TimeDelta", init<>())
      .def(init<int64_t>())
      .def(init<int64_t, int64_t>())
      .def(init<int64_t, int64_t, int64_t>())
      .def(init<int64_t, int64_t, int64_t, int64_t>())
      .def(init<int64_t, int64_t, int64_t, int64_t, int64_t>())
      .def(init<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>())
      .def(self_ns::str(self))
      .add_property("days", &TimeDelta::days)
      .add_property("hours", &TimeDelta::hours)
      .add_property("minutes", &TimeDelta::minutes)
      .add_property("seconds", &TimeDelta::seconds)
      .add_property("milliseconds", &TimeDelta::milliseconds)
      .add_property("microseconds", &TimeDelta::microseconds)
      .add_property("ticks", &TimeDelta::ticks)
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

      .def("__add__", &TimeDelta::operator+)
      .def("__sub__", &TimeDelta::operator-)

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TimeDelta>())
#endif
      ;
}
