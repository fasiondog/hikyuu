/*
 * _Performance.cpp
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/Performance.h>

using namespace boost::python;
using namespace hku;

void export_Performance() {
    class_<Performance>("Performance", init<>())
            .def("reset", &Performance::reset)
            .def("report", &Performance::report,
                    (arg("tm"), arg("datetime")=Datetime::now()))
            .def("statistics", &Performance::statistics,
                    (arg("tm"), arg("datetime")=Datetime::now()))
            .def("get", &Performance::get)
            .def("__getitem__", &Performance::get)
            ;
}



