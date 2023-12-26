/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

static std::string TimeLineRecord_to_str(const TimeLineRecord& record) {
    std::stringstream out;
    out << record;
    return out.str();
}

void export_TimeLineReord(py::module& m) {
    py::class_<TimeLineRecord>(m, "TimeLineRecord", "分时线记录，属性可读写")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t>())
      .def("__str__", TimeLineRecord_to_str)
      .def("__repr__", TimeLineRecord_to_str)
      .def_readwrite("date", &TimeLineRecord::datetime, "日期时间")
      .def_readwrite("price", &TimeLineRecord::price, "价格")
      .def_readwrite("vol", &TimeLineRecord::vol, "成交量")
      .def(py::self == py::self)

        DEF_PICKLE(TimeLineRecord);

    //     class_<TimeLineList>("TimeLineList")
    //       .def(vector_indexing_suite<TimeLineList>())
    // #if HKU_PYTHON_SUPPORT_PICKLE
    //       .def_pickle(normal_pickle_suite<TimeLineList>())
    // #endif
    //       ;
}
