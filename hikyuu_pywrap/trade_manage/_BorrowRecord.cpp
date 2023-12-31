/*
 * _BorrowRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/BorrowRecord.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_BorrowRecord(py::module& m) {
    py::class_<BorrowRecord>(m, "BorrowRecord", "记录当前借入的股票信息")
      .def(py::init<>())
      .def(py::init<const Stock&, double, price_t>())
      .def("__str__", to_py_str<BorrowRecord>)
      .def("__repr__", to_py_str<BorrowRecord>)
      .def_readwrite("stock", &BorrowRecord::stock, "借入的证券")
      .def_readwrite("number", &BorrowRecord::number, "借入总数量")
      .def_readwrite("value", &BorrowRecord::value, "借入总价值")

        DEF_PICKLE(BorrowRecord);
}
