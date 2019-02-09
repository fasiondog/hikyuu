/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*record_eq)(const TimeLineRecord&, const TimeLineRecord&) = operator==;

void export_TimeLineReord() {
    class_<TimeLineRecord>("TimeLineRecord", init<>())
            .def(init<const Datetime&, price_t, price_t>())
            .def(self_ns::str(self))
            .def_readwrite("datetime", &TimeLineRecord::datetime)
            .def_readwrite("price", &TimeLineRecord::price)
            .def_readwrite("vol", &TimeLineRecord::vol)
            .def("__eq__", record_eq)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TimeLineRecord>())
#endif
            ;

    TimeLine::const_reference (TimeLine::*TimeLine_at)(TimeLine::size_type) const = &TimeLine::at;
    void (TimeLine::*append)(const TimeLineRecord&) = &TimeLine::push_back;
    class_<TimeLine>("TimeLine")
            .def(self_ns::str(self))
            .def("__iter__", iterator<TimeLine>())
            .def("size", &TimeLine::size)
            .def("__len__", &TimeLine::size)
            .def("get", TimeLine_at, return_value_policy<copy_const_reference>())
            .def("append", append)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TimeLine>())
#endif
            ;

    register_ptr_to_python<TimeLinePtr>();
}

