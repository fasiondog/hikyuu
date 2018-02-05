/*
 * _Datetime.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/Datetime_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*eq)(const Datetime&, const Datetime&) = operator==;
bool (*ne)(const Datetime&, const Datetime&) = operator!=;
bool (*gt)(const Datetime&, const Datetime&) = operator>;
bool (*lt)(const Datetime&, const Datetime&) = operator<;
bool (*ge)(const Datetime&, const Datetime&) = operator>=;
bool (*le)(const Datetime&, const Datetime&) = operator<=;

void export_Datetime() {
    docstring_options doc_options(false, true, false);

    class_<Datetime>("Datetime")
            .def(init<unsigned long long>())
            .def(init<const std::string&>())
            .def(self_ns::str(self))
            .add_property("year", &Datetime::year)
            .add_property("month", &Datetime::month)
            .add_property("day", &Datetime::day)
            .add_property("hour", &Datetime::hour)
            .add_property("minute", &Datetime::minute)
            .add_property("second", &Datetime::second)
            .add_property("number", &Datetime::number)

            .def("toString", &Datetime::toString)
            .def("dayOfWeek", &Datetime::dayOfWeek)
            .def("dayOfYear", &Datetime::dayOfYear)
            .def("endOfMonth", &Datetime::endOfMonth)
            .def("nextDay", &Datetime::nextDay)
            .def("min", &Datetime::min).staticmethod("min")
            .def("max", &Datetime::max).staticmethod("max")
            .def("now", &Datetime::now).staticmethod("now")
            //不支持boost.date
            //.def("date", &Datetime::date)
            .def("__eq__", eq)
            .def("__ne__", ne)
            .def("__gt__", gt)
            .def("__lt__", lt)
            .def("__ge__", ge)
            .def("__le__", le)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<Datetime>())
#endif
            ;


    def("getDateRange", getDateRange, (arg("start"), arg("end")));
}

