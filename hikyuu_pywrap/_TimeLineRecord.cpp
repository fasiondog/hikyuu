/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "pybind_utils.h"
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TimeLineReord() {
    class_<TimeLineRecord>("TimeLineRecord", "分时线记录，属性可读写", init<>())
      .def(init<const Datetime&, price_t, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .def_readwrite("date", &TimeLineRecord::datetime, "日期时间")
      .def_readwrite("price", &TimeLineRecord::price, "价格")
      .def_readwrite("vol", &TimeLineRecord::vol, "成交量")
      .def(self == self)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TimeLineRecord>())
#endif
      ;

    class_<TimeLineList>("TimeLineList")
      .def(vector_indexing_suite<TimeLineList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TimeLineList>())
#endif
      ;
}
