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
    py::class_<BorrowRecord>(m, "BorrowRecord", "Record the currently borrowed stock information")
      .def(py::init<>())
      .def(py::init<const Stock&, double, price_t>())
      .def("__str__", to_py_str<BorrowRecord>)
      .def("__repr__", to_py_str<BorrowRecord>)
      .def_readwrite("stock", &BorrowRecord::stock, "The borrowed security")
      .def_readwrite("number", &BorrowRecord::number, "The total borrowed quantity")
      .def_readwrite("value", &BorrowRecord::value, "The total borrowed value")

        DEF_PICKLE(BorrowRecord);
}
