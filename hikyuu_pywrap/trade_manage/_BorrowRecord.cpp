/*
 * _BorrowRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/BorrowRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_BorrowRecord() {
    class_<BorrowRecord>("BorrowRecord", "记录当前借入的股票信息", init<>())
      .def(init<const Stock&, double, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def_readwrite("stock", &BorrowRecord::stock, "借入的证券")
      .def_readwrite("number", &BorrowRecord::number, "借入总数量")
      .def_readwrite("value", &BorrowRecord::value, "借入总价值")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<BorrowRecord>())
#endif
      ;

    BorrowRecordList::const_reference (BorrowRecordList::*BorrowRecordList_at)(
      BorrowRecordList::size_type) const = &BorrowRecordList::at;
    class_<BorrowRecordList>("BorrowRecordList")
      .def("__iter__", iterator<BorrowRecordList>())
      .def("size", &BorrowRecordList::size)
      .def("__len__", &BorrowRecordList::size)
      .def("__getitem__", BorrowRecordList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<BorrowRecordList>())
#endif
      ;
}
