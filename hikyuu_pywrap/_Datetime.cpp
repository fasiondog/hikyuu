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
                         R"(The date-time class (accurate to the microsecond), built in the following ways:
    
    - From a string: Datetime("2010-1-1 10:00:00"), Datetime("2001-1-1"),
                 Datetime("20010101")、Datetime("20010101T232359)
    - From a Python date: Datetime(date(2010,1,1))
    - From a Python datetime: Datetime(datetime(2010,1,1,10)
    - From an integer in the YYYYMMDDHHMMSS, YYYYMMDDHHMM or YYYYMMDD form: Datetime(201001011000), Datetime(20010101)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0))")

      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<unsigned long long>())
      .def(py::init<const Datetime&>())
      .def(py::init<long, long, long, long, long, long, long, long>(), py::arg("year"),
           py::arg("month"), py::arg("day"), py::arg("hour") = 0, py::arg("minute") = 0,
           py::arg("second") = 0, py::arg("millisecond") = 0, py::arg("microsecond") = 0)
      .def("__init__",
           [](Datetime& self, const py::object& source) { self = pydatetime_to_Datetime(source); })

      .def("__str__", &Datetime::str)
      .def("__repr__", &Datetime::repr)

      .def_property_readonly("year", &Datetime::year, "Year")
      .def_property_readonly("month", &Datetime::month, "Month")
      .def_property_readonly("day", &Datetime::day, "Day")
      .def_property_readonly("hour", &Datetime::hour, "Hour")
      .def_property_readonly("minute", &Datetime::minute, "Minute")
      .def_property_readonly("second", &Datetime::second, "Second")
      .def_property_readonly("millisecond", &Datetime::millisecond, "Millisecond")
      .def_property_readonly("microsecond", &Datetime::microsecond, "Microsecond")
      .def_property_readonly("number", &Datetime::number, "Return the number displayed as YYYYMMDDhhmm")
      .def_property_readonly("hex", &Datetime::hex,
                             "Return a 64-bit integer in which the last 7 bytes represent the century, the century year, the month, the day, the hour, the minute and the second")
      .def_property_readonly("ym", &Datetime::ym, "Return the number displayed as YYYYMM")
      .def_property_readonly("ymd", &Datetime::ymd, "Return the number displayed as YYYYMMDD")
      .def_property_readonly("ymdh", &Datetime::ymdh, "Return the number displayed as YYYYMMDDhh")
      .def_property_readonly("ymdhm", &Datetime::ymdhm, "Return the number displayed as YYYYMMDDhhmm")
      .def_property_readonly("ymdhms", &Datetime::ymdhms, "Return the number displayed as YYYYMMDDhhmmss")
      .def_property_readonly("ticks", &Datetime::ticks, "Return the microseconds elapsed since the minimum date")

      .def("is_null", &Datetime::isNull, "\nWhether it is a Null value, equal to the object created directly by Datetime()")

      .def("day_of_week", &Datetime::dayOfWeek, "\nReturn the day of the week; Sunday is 0 and Monday is 1")
      .def("day_of_year", &Datetime::dayOfYear, "\nReturn the day of the year; January 1st is the 1st day of the year")
      .def("start_of_day", &Datetime::startOfDay, "\nReturn 00:00:00 of the current day")
      .def("end_of_day", &Datetime::endOfDay, "\nReturn 23:59:59 of the current day")
      .def("next_day", &Datetime::nextDay, "\nReturn the next natural day")
      .def("next_week", &Datetime::nextWeek, "\nReturn the Monday date of the next week")
      .def("next_month", &Datetime::nextMonth, "\nReturn the first day of the next month")
      .def("next_quarter", &Datetime::nextQuarter, "\nReturn the first day of the next quarter")
      .def("next_halfyear", &Datetime::nextHalfyear, "\nReturn the first day of the next half-year")
      .def("next_year", &Datetime::nextYear, "\nReturn the first day of the next year")
      .def("pre_day", &Datetime::preDay, "\nReturn the previous natural day")
      .def("pre_week", &Datetime::preWeek, "\nReturn the Monday date of the previous week")
      .def("pre_month", &Datetime::preMonth, "\nReturn the first day of the previous month")
      .def("pre_quarter", &Datetime::preQuarter, "\nReturn the first day of the previous quarter")
      .def("pre_halfyear", &Datetime::preHalfyear, "\nReturn the first day of the previous half-year")
      .def("pre_year", &Datetime::preYear, "\nReturn the first day of the previous year")
      .def("date_of_week", &Datetime::dateOfWeek,
           R"(
    Return the date of the specified day of this week; Sunday is day 0 and Saturday is day 6

    :param int day: indicate the day of this week; if it is less than 0, it is considered day 0, and if it is greater than 6, it is considered day 6)")

      .def("start_of_week", &Datetime::startOfWeek, "\nReturn the start date of the week (Monday)")
      .def("end_of_week", &Datetime::endOfWeek, "\nReturn the end date of the week (Sunday)")
      .def("start_of_month", &Datetime::startOfMonth, "\nReturn the start date of the month")
      .def("end_of_month", &Datetime::endOfMonth, "\nReturn the last day of the month")
      .def("start_of_quarter", &Datetime::startOfQuarter, "\nReturn the start date of the quarter")
      .def("end_of_quarter", &Datetime::endOfQuarter, "\nReturn the end date of the quarter")
      .def("start_of_halfyear", &Datetime::startOfHalfyear, "\nReturn the start date of the half-year")
      .def("end_of_halfyear", &Datetime::endOfHalfyear, "\nReturn the end date of the half-year")
      .def("start_of_year", &Datetime::startOfYear, "\nReturn the start date of the year")
      .def("endOfYear", &Datetime::endOfYear, "\nReturn the end date of the year")
      .def("timestamp", &Datetime::timestamp, "\nReturn the timestamp (at the microsecond level)")
      .def("timestamp_utc", &Datetime::timestampUTC,
           "\nReturn the timestamp (at the microsecond level), deducting the local UTC offset time")
      .def_static("min", &Datetime::min, "\nGet the minimum supported date, Datetime(1400, 1, 1)")
      .def_static("max", &Datetime::max, "\nGet the maximum supported date, Datetime(9999, 12, 31)")
      .def_static("now", &Datetime::now, "\nGet the current system date-time")
      .def_static("today", &Datetime::today, "\nGet the current date")
      .def_static("from_hex", &Datetime::fromHex, "\nCompatible with the oracle datetime represented by the last 7 bytes")
      .def_static("from_timestamp", &Datetime::fromTimestamp, "\nCreate a Datetime object from the timestamp (in microseconds)")
      .def_static("from_timestamp_utc", &Datetime::fromTimestampUTC,
                  "\nCreate a Datetime object from the timestamp (in microseconds), adding the local UTC offset")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self < py::self)

      .def(py::self - py::self)

      .def(py::self + TimeDelta())
      //.def(other<TimeDelta>() + self) extended and supported in python
      .def(py::self - TimeDelta())

        DEF_PICKLE(Datetime);

    m.def("get_date_range", getDateRange, py::arg("start"), py::arg("end"),
          R"(get_date_range(start, end)

    Get the list of the natural calendar dates in the specified [start, end) date-time range, supported only up to the day
    Note: if the end date is empty, the maximum date of Datetime will be used, which may use excessive memory
    
    :param Datetime start: the start date
    :param Datetime end: the end date
    :rtype: DatetimeList)");

    m.def(
      "dates_to_np",
      [](const DatetimeList& datelist) {
          size_t total = datelist.size();
          HKU_IF_RETURN(total == 0, py::array());

          // Allocate the memory with malloc
          int64_t* data = static_cast<int64_t*>(std::malloc(total * sizeof(int64_t)));
          for (size_t i = 0; i < total; i++) {
              data[i] = datelist[i].timestamp() * 1000LL;
          }

          // Define the NumPy structured data type
          py::dtype dtype;
          dtype = py::dtype(vector_to_python_list<string>({"datetime"}),
                            vector_to_python_list<string>({"datetime64[ns]"}),
                            vector_to_python_list<int64_t>({0}), 8);

          // Manage the memory with the capsule
          return py::array(dtype, total, data, py::capsule(data, [](void* p) { std::free(p); }));
      },
      "Convert a DatetimeList to a NumPy tuple");
}
