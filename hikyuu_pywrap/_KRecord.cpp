/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KRecord_serialization.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

bool (*krecord_eq)(const KRecord&, const KRecord&) = operator==;
bool (*krecord_ne)(const KRecord&, const KRecord&) = operator!=;

void export_KReord() {
    class_<KRecord>("KRecord", "K线记录，组成K线数据，属性可读写", init<>())
      .def(init<const Datetime&>())
      .def(init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def_readwrite("datetime", &KRecord::datetime, "时间")
      .def_readwrite("open", &KRecord::openPrice, "开盘价")
      .def_readwrite("high", &KRecord::highPrice, "最高价")
      .def_readwrite("low", &KRecord::lowPrice, "最低价")
      .def_readwrite("close", &KRecord::closePrice, "收盘价")
      .def_readwrite("amount", &KRecord::transAmount, "成交金额")
      .def_readwrite("volume", &KRecord::transCount, "成交量")

      .def("__eq__", krecord_eq)
      .def("__ne__", krecord_ne)

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<KRecord>())
#endif
      ;

    class_<KRecordList>("KRecordList").def(vector_indexing_suite<KRecordList>());

    register_ptr_to_python<KRecordListPtr>();
}
