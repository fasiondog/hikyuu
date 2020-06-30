/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include "pybind_utils.h"
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

    VECTOR_TO_PYTHON_CONVERT(TimeLineList);
}
