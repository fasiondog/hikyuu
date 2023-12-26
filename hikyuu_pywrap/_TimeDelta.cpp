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
                          R"(时间时长，用于时间计算。可通过以下方式构建：

    - 通过 datetime.timedelta 构建。TimdeDelta(timedelta实例)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    以上参数限制，主要为防止求总微秒数时可能出现溢出的情况。如只使用一个参数不希望存在上述限制时，可使用快捷函数：
    Days, Hours, Minutes, Seconds, Milliseconds, Microseconds)")

      .def(py::init<>())
      .def(py::init<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(), py::arg("days") = 0,
           py::arg("hours") = 0, py::arg("minutes") = 0, py::arg("seconds") = 0,
           py::arg("milliseconds") = 0, py::arg("microseconds") = 0)

      .def("__str__", &TimeDelta::str)
      .def("__repr__", &TimeDelta::repr)

      .def_property_readonly("days", &TimeDelta::days, "天数 [-99999999, 99999999]")
      .def_property_readonly("hours", &TimeDelta::hours, "小时数 [0, 23]")
      .def_property_readonly("minutes", &TimeDelta::minutes, "分钟数 [0, 59]")
      .def_property_readonly("seconds", &TimeDelta::seconds, "秒数 [0, 59]")
      .def_property_readonly("milliseconds", &TimeDelta::milliseconds, "毫秒数 [0, 999]")
      .def_property_readonly("microseconds", &TimeDelta::microseconds, "微秒数 [0, 999]")
      .def_property_readonly("ticks", &TimeDelta::ticks, "同总微秒数")

      .def("isNegative", &TimeDelta::isNegative, R"(isNegative(self)

    是否为负时长

    :rtype: bool)")

      .def("total_days", &TimeDelta::total_days, R"(total_days(self)

    获取带小数的总天数

    :rtype: float)")

      .def("total_hours", &TimeDelta::total_hours, R"(total_hours(self)

    获取带小数的总小时数

    :rtype: float)")

      .def("total_minutes", &TimeDelta::total_minutes, R"(total_minutes(self)

    获取带小数的总分钟数

    :rtype: float)")

      .def("total_seconds", &TimeDelta::total_seconds, R"(total_seconds(self)

    获取带小数的总秒数

    :rtype: float)")

      .def("total_milliseconds", &TimeDelta::total_milliseconds, R"(total_milliseconds(self)

    获取带小数的总毫秒数

    :rtype: float)")

      .def("max", &TimeDelta::max, R"(max()

    支持的最大时长

    :return: TimeDelta(99999999, 23, 59, 59, 999, 999))")

      .def("min", &TimeDelta::min, R"(min()

    支持的最小时长

    :return: TimeDelta(-99999999, 0, 0, 0, 0, 0))")

      .def("resolution", &TimeDelta::resolution, R"(resolution()

    支持的最小精度
        
    :return: TimeDelta(0, 0, 0, 0, 0, 1))")

      .def("max_ticks", &TimeDelta::maxTicks, R"(max_ticks()

    支持的最大 ticks （即微秒数）

    :rtype: int)")

      .def("min_ticks", &TimeDelta::minTicks, R"(min_ticks()

    支持的最小 ticks （即微秒数）

    :rtype: int)")

      .def("from_ticks", &TimeDelta::fromTicks, R"(from_ticks(ticks)

    使用 ticks（即微秒数） 值创建

    :param int ticks: 微秒数
    :rtype: TimeDelta)")

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

      TimeDelta 快捷创建函数

      :param int days: 天数 [-99999999, 99999999]
      :rtype: TimeDelta)");

    m.def("Hours", Hours, R"(Hours(hours)

      TimeDelta 快捷创建函数

      :param int hours: 小时数
      :rtype: TimeDelta)");

    m.def("Minutes", Minutes, R"(Minutes(mins)

      TimeDelta 快捷创建函数

      :param int mins: 分钟数
      :rtype: TimeDelta)");

    m.def("Seconds", Seconds, R"(Seconds(secs)

      TimeDelta 快捷创建函数

      :param int secs: 秒数
      :rtype: TimeDelta)");

    m.def("Milliseconds", Milliseconds, R"(Milliseconds(milliseconds)

      TimeDelta 快捷创建函数

      :param int milliseconds: 毫秒数
      :rtype: TimeDelta)");

    m.def("Microseconds", Microseconds, R"(Microseconds(microsecs)

      TimeDelta 快捷创建函数

      :param int microsecs: 微秒数
      :rtype: TimeDelta)");
}
