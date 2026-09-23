/*
 * _TimeDelta.cpp
 *
 *  Copyright (C) 2019 hikyuu.org
 *
 *  Created on: 2019-12-14
 *      Author: fasiondog
 */

#include <hikyuu/serialization/TimeDelta_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

double (TimeDelta::*TimeDelta_div_1)(TimeDelta) const = &TimeDelta::operator/;
TimeDelta (TimeDelta::*TimeDelta_div_2)(double) const = &TimeDelta::operator/;

TimeDelta (TimeDelta::*TimeDelta_pos)() const = &TimeDelta::operator+;
TimeDelta (TimeDelta::*TimeDelta_neg)() const = &TimeDelta::operator-;

void export_TimeDelta(py::module& m) {
    py::class_<TimeDelta>(m, "TimeDelta",
                          R"(The time duration, used for the time calculation. It can be built in the following ways:

    - Build from a datetime.timedelta. TimeDelta(a timedelta instance)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    The parameter limits above are mainly to prevent a possible overflow when summing the total microseconds. When only one parameter is used and the limits above are not desired, the shortcut functions can be used:
    Days, Hours, Minutes, Seconds, Milliseconds, Microseconds)")

      .def(py::init<>())
      .def(py::init<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(), py::arg("days") = 0,
           py::arg("hours") = 0, py::arg("minutes") = 0, py::arg("seconds") = 0,
           py::arg("milliseconds") = 0, py::arg("microseconds") = 0)

      .def("__str__", &TimeDelta::str)
      .def("__repr__", &TimeDelta::repr)

      .def_property_readonly("days", &TimeDelta::days, "The number of days [-99999999, 99999999]")
      .def_property_readonly("hours", &TimeDelta::hours, "The number of hours [0, 23]")
      .def_property_readonly("minutes", &TimeDelta::minutes, "The number of minutes [0, 59]")
      .def_property_readonly("seconds", &TimeDelta::seconds, "The number of seconds [0, 59]")
      .def_property_readonly("milliseconds", &TimeDelta::milliseconds, "The number of milliseconds [0, 999]")
      .def_property_readonly("microseconds", &TimeDelta::microseconds, "The number of microseconds [0, 999]")
      .def_property_readonly("ticks", &TimeDelta::ticks, "The same as the total microseconds")

      .def("isNegative", &TimeDelta::isNegative, R"(isNegative(self)

    Whether it is a negative duration

    :rtype: bool)")

      .def("total_days", &TimeDelta::total_days, R"(total_days(self)

    Get the total number of days with decimals

    :rtype: float)")

      .def("total_hours", &TimeDelta::total_hours, R"(total_hours(self)

    Get the total number of hours with decimals

    :rtype: float)")

      .def("total_minutes", &TimeDelta::total_minutes, R"(total_minutes(self)

    Get the total number of minutes with decimals

    :rtype: float)")

      .def("total_seconds", &TimeDelta::total_seconds, R"(total_seconds(self)

    Get the total number of seconds with decimals

    :rtype: float)")

      .def("total_milliseconds", &TimeDelta::total_milliseconds, R"(total_milliseconds(self)

    Get the total number of milliseconds with decimals

    :rtype: float)")

      .def("max", &TimeDelta::max, R"(max()

    The maximum supported duration

    :return: TimeDelta(99999999, 23, 59, 59, 999, 999))")

      .def("min", &TimeDelta::min, R"(min()

    The minimum supported duration

    :return: TimeDelta(-99999999, 0, 0, 0, 0, 0))")

      .def("resolution", &TimeDelta::resolution, R"(resolution()

    The minimum supported precision
        
    :return: TimeDelta(0, 0, 0, 0, 0, 1))")

      .def("max_ticks", &TimeDelta::maxTicks, R"(max_ticks()

    The maximum supported ticks (i.e. the number of microseconds)

    :rtype: int)")

      .def("min_ticks", &TimeDelta::minTicks, R"(min_ticks()

    The minimum supported ticks (i.e. the number of microseconds)

    :rtype: int)")

      .def("from_ticks", &TimeDelta::fromTicks, R"(from_ticks(ticks)

    Create with the ticks (i.e. the number of microseconds) value

    :param int ticks: the number of microseconds
    :rtype: TimeDelta)")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self < py::self)
      .def(-py::self)
      .def(+py::self)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self % py::self)
      .def(py::self * float())

      .def("__abs__", &TimeDelta::abs)
      .def("__rmul__", &TimeDelta::operator*)
      .def("__truediv__", TimeDelta_div_1)
      .def("__truediv__", TimeDelta_div_2)
      .def("__floordiv__", &TimeDelta::floorDiv)

        DEF_PICKLE(TimeDelta);

    m.def("Days", Days, R"(Days(days)

      The TimeDelta shortcut creation function

      :param int days: the number of days [-99999999, 99999999]
      :rtype: TimeDelta)");

    m.def("Hours", Hours, R"(Hours(hours)

      The TimeDelta shortcut creation function

      :param int hours: the number of hours
      :rtype: TimeDelta)");

    m.def("Minutes", Minutes, R"(Minutes(mins)

      The TimeDelta shortcut creation function

      :param int mins: the number of minutes
      :rtype: TimeDelta)");

    m.def("Seconds", Seconds, R"(Seconds(secs)

      The TimeDelta shortcut creation function

      :param int secs: the number of seconds
      :rtype: TimeDelta)");

    m.def("Milliseconds", Milliseconds, R"(Milliseconds(milliseconds)

      The TimeDelta shortcut creation function

      :param int milliseconds: the number of milliseconds
      :rtype: TimeDelta)");

    m.def("Microseconds", Microseconds, R"(Microseconds(microsecs)

      The TimeDelta shortcut creation function

      :param int microsecs: the number of microseconds
      :rtype: TimeDelta)");

    m.def("UTCOffset", UTCOffset, R"(UTCOffset()

      Get the current system UTC offset

      :rtype: TimeDelta)");
}
