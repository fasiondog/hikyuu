/*
 * _Datetime.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <hikyuu/serialization/Datetime_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Datetime(py::module& m) {
    py::class_<Datetime>(m, "Datetime",
                         R"(日期时间类（精确到微秒），通过以下方式构建：
    
    - 通过字符串：Datetime("2010-1-1 10:00:00")、Datetime("2001-1-1")、
                 Datetime("20010101")、Datetime("20010101T232359)
    - 通过 Python 的date：Datetime(date(2010,1,1))
    - 通过 Python 的datetime：Datetime(datetime(2010,1,1,10)
    - 通过 YYYYMMDDHHMMss 或 YYYYMMDDHHMM 或 YYYYMMDD 形式的整数：Datetime(201001011000)、Datetime(20010101)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0))")

      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<unsigned long long>())
      .def(py::init<long, long, long, long, long, long, long, long>(), py::arg("year"),
           py::arg("month"), py::arg("day"), py::arg("hour") = 0, py::arg("minute") = 0,
           py::arg("second") = 0, py::arg("millisecond") = 0, py::arg("microsecond") = 0)

      .def("__str__", &Datetime::str)
      .def("__repr__", &Datetime::repr)

      .def_property_readonly("year", &Datetime::year, "年")
      .def_property_readonly("month", &Datetime::month, "月")
      .def_property_readonly("day", &Datetime::day, "日")
      .def_property_readonly("hour", &Datetime::hour, "时")
      .def_property_readonly("minute", &Datetime::minute, "分")
      .def_property_readonly("second", &Datetime::second, "秒")
      .def_property_readonly("millisecond", &Datetime::millisecond, "毫秒")
      .def_property_readonly("microsecond", &Datetime::microsecond, "微秒")
      .def_property_readonly("number", &Datetime::number, "返回显示如 YYYYMMDDhhmm 的数字")
      .def_property_readonly("hex", &Datetime::hex,
                             "返回用后7个字节表示世纪、世纪年、月、日、时、分、秒的64位整数")
      .def_property_readonly("ym", &Datetime::ym, "返回显示如 YYYYMM 的数字")
      .def_property_readonly("ymd", &Datetime::ymd, "返回显示如 YYYYMMDD 的数字")
      .def_property_readonly("ymdh", &Datetime::ymdh, "返回显示如 YYYYMMDDhh 的数字")
      .def_property_readonly("ymdhm", &Datetime::ymdhm, "返回显示如 YYYYMMDDhhmm 的数字")
      .def_property_readonly("ymdhms", &Datetime::ymdhms, "返回显示如 YYYYMMDDhhmms 的数字")
      .def_property_readonly("ticks", &Datetime::ticks, "返回距离最小日期过去的微秒数")

      .def("is_null", &Datetime::isNull, "\n是否是Null值，等于 Datetime() 直接创建的对象")

      .def("day_of_week", &Datetime::dayOfWeek, "\n返回是一周中的第几天，周日为0，周一为1")
      .def("day_of_year", &Datetime::dayOfYear, "\n返回一年中的第几天，1月1日为一年中的第1天")
      .def("start_of_day", &Datetime::startOfDay, "\n返回当天 0点0分0秒")
      .def("end_of_day", &Datetime::endOfDay, "\n返回当日 23点59分59秒")
      .def("next_day", &Datetime::nextDay, "\n返回下一自然日")
      .def("next_week", &Datetime::nextWeek, "\n返回下周周一日期")
      .def("next_month", &Datetime::nextMonth, "\n返回下月首日日期")
      .def("next_quarter", &Datetime::nextQuarter, "\n返回下一季度首日日期")
      .def("next_halfyear", &Datetime::nextHalfyear, "\n返回下一半年度首日日期")
      .def("next_year", &Datetime::nextYear, "\n返回下一年度首日日期")
      .def("pre_day", &Datetime::preDay, "\n返回前一自然日日期")
      .def("pre_week", &Datetime::preWeek, "\n返回上周周一日期")
      .def("pre_month", &Datetime::preMonth, "\n返回上月首日日期")
      .def("pre_quarter", &Datetime::preQuarter, "\n返回上一季度首日日期")
      .def("pre_halfyear", &Datetime::preHalfyear, "\n返回上一半年度首日日期")
      .def("pre_year", &Datetime::preYear, "\n返回上一年度首日日期")
      .def("date_of_week", &Datetime::dateOfWeek,
           R"(
    返回指定的本周中第几天的日期，周日为0天，周六为第6天

    :param int day: 指明本周的第几天，如小于则认为为第0天，如大于6则认为为第6天)")

      .def("start_of_week", &Datetime::startOfWeek, "\n返回周起始日期（周一）")
      .def("end_of_week", &Datetime::endOfWeek, "\n返回周结束日期（周日）")
      .def("start_of_month", &Datetime::startOfMonth, "\n返回月度起始日期")
      .def("end_of_month", &Datetime::endOfMonth, "\n返回月末最后一天日期")
      .def("start_of_quarter", &Datetime::startOfQuarter, "\n返回季度起始日期")
      .def("end_of_quarter", &Datetime::endOfQuarter, "\n返回季度结束日期")
      .def("start_of_halfyear", &Datetime::startOfHalfyear, "\n返回半年度起始日期")
      .def("end_of_halfyear", &Datetime::endOfHalfyear, "\n返回半年度结束日期")
      .def("start_of_year", &Datetime::startOfYear, "\n返回年度起始日期")
      .def("endOfYear", &Datetime::endOfYear, "\n返回年度结束日期")
      .def_static("min", &Datetime::min, "\n获取支持的最小日期, Datetime(1400, 1, 1)")
      .def_static("max", &Datetime::max, "\n获取支持的最大日期, Datetime(9999, 12, 31)")
      .def_static("now", &Datetime::now, "\n获取系统当前日期时间")
      .def_static("today", &Datetime::today, "\n获取当前的日期")
      .def_static("from_hex", &Datetime::fromHex, "\n兼容oracle用后7个字节表示的datetime")

      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self < py::self)

      .def(py::self - py::self)

      .def(py::self + TimeDelta())
      //.def(other<TimeDelta>() + self) 在python里扩展支持
      .def(py::self - TimeDelta())

        DEF_PICKLE(Datetime);

    m.def("get_date_range", getDateRange, py::arg("start"), py::arg("end"));
}
