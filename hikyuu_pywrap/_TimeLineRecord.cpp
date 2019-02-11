/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*timelinerecord_eq)(const TimeLineRecord&, const TimeLineRecord&) = operator==;

void export_TimeLineReord() {
    class_<TimeLineRecord>("TimeLineRecord", init<>())
            .def(init<const Datetime&, price_t, price_t>())
            .def(self_ns::str(self))
            .def_readwrite("datetime", &TimeLineRecord::datetime)
            .def_readwrite("price", &TimeLineRecord::price)
            .def_readwrite("vol", &TimeLineRecord::vol)
            .def("__eq__", timelinerecord_eq)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TimeLineRecord>())
#endif
            ;

    TimeLineList::const_reference (TimeLineList::*TimeLine_at)(TimeLineList::size_type) const = &TimeLineList::at;
    void (TimeLineList::*append)(const TimeLineRecord&) = &TimeLineList::push_back;
    class_<TimeLineList>("TimeLineList")
            .def(self_ns::str(self))
            .def("__iter__", iterator<TimeLineList>())
            .def("size", &TimeLineList::size)
            .def("__len__", &TimeLineList::size)
            .def("get", TimeLine_at, return_value_policy<copy_const_reference>())
            .def("append", append)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TimeLineList>())
#endif
            ;

    register_ptr_to_python<TimeLineListPtr>();
}

