/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/KRecord_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

bool (*krecord_eq)(const KRecord&, const KRecord&) = operator==;
bool (*krecord_ne)(const KRecord&, const KRecord&) = operator!=;

void export_KReord(py::module& m) {
    py::class_<KRecord>(m, "KRecord", "K线记录，组成K线数据，属性可读写")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())

      .def("__str__", to_py_str<KRecord>)
      .def("__repr__", to_py_str<KRecord>)

      .def_readwrite("datetime", &KRecord::datetime, "时间")
      .def_readwrite("open", &KRecord::openPrice, "开盘价")
      .def_readwrite("high", &KRecord::highPrice, "最高价")
      .def_readwrite("low", &KRecord::lowPrice, "最低价")
      .def_readwrite("close", &KRecord::closePrice, "收盘价")
      .def_readwrite("amount", &KRecord::transAmount, "成交金额")
      .def_readwrite("volume", &KRecord::transCount, "成交量")

      .def("__eq__", krecord_eq)
      .def("__ne__", krecord_ne)

        DEF_PICKLE(KRecord);
}
