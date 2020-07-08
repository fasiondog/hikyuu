/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KRecord_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*krecord_eq)(const KRecord&, const KRecord&) = operator==;

void export_KReord() {
    class_<KRecord>("KRecord", "K线记录，组成K线数据，属性可读写", init<>())
      .def(init<const Datetime&>())
      .def(init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def_readwrite("date", &KRecord::datetime, "时间")
      .def_readwrite("open", &KRecord::openPrice, "开盘价")
      .def_readwrite("high", &KRecord::highPrice, "最高价")
      .def_readwrite("low", &KRecord::lowPrice, "最低价")
      .def_readwrite("close", &KRecord::closePrice, "收盘价")
      .def_readwrite("amount", &KRecord::transAmount, "成交金额")
      .def_readwrite("volume", &KRecord::transCount, "成交量")

      .def("__eq__", krecord_eq)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<KRecord>())
#endif
      ;

    KRecordList::const_reference (KRecordList::*KRecordList_at)(KRecordList::size_type) const =
      &KRecordList::at;
    void (KRecordList::*append)(const KRecord&) = &KRecordList::push_back;
    class_<KRecordList>("KRecordList")
      .def("__iter__", iterator<KRecordList>())
      .def("size", &KRecordList::size)
      .def("__len__", &KRecordList::size)
      .def("__getitem__", KRecordList_at, return_value_policy<copy_const_reference>())
      .def("append", append);

    register_ptr_to_python<KRecordListPtr>();
}
