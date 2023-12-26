/*
 * _LoanRecord.cpp
 *
 *  Created on: 2013-5-24
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/LoanRecord.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_LoanRecord(py::module& m) {
    py::class_<LoanRecord>(m, "LoanRecord", "借款记录（融资记录）")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t>())
      .def("__str__", to_py_str<LoanRecord>)
      .def("__repr__", to_py_str<LoanRecord>)
      .def_readwrite("datetime", &LoanRecord::datetime, "借款时间")
      .def_readwrite("value", &LoanRecord::value, "借款金额")

        DEF_PICKLE(LoanRecord);
}
