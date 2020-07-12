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

void export_TimeLineReord() {
    class_<TimeLineRecord>("TimeLineRecord", "分时线记录，属性可读写", init<>())
      .def(init<const Datetime&, price_t, price_t>())
      .def(self_ns::str(self))
      .def_readwrite("datetime", &TimeLineRecord::datetime, "日期时间")
      .def_readwrite("price", &TimeLineRecord::price, "价格")
      .def_readwrite("vol", &TimeLineRecord::vol, "成交量")
      .def(self == self)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TimeLineRecord>())
#endif
      ;

    TimeLineList::const_reference (TimeLineList::*TimeLine_at)(TimeLineList::size_type) const =
      &TimeLineList::at;
    void (TimeLineList::*append)(const TimeLineRecord&) = &TimeLineList::push_back;
    class_<TimeLineList>("TimeLineList", "由 TimeLineRecord 组成的数组，可象 list 一样进行遍历")
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
}
