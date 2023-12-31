/*
 * _KRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <hikyuu/serialization/TransRecord_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TransRecord(py::module& m) {
    py::class_<TransRecord>(m, "TransRecord")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t, TransRecord::DIRECT>())
      .def("__str__", to_py_str<TransRecord>)
      .def("__repr__", to_py_str<TransRecord>)
      .def_readwrite("date", &TransRecord::datetime, "时间")
      .def_readwrite("price", &TransRecord::price, "价格")
      .def_readwrite("vol", &TransRecord::vol, "成交量")
      .def_readwrite("direct", &TransRecord::direct, "买卖盘性质, 参见: TransRecord.DIRECT")
      .def(py::self == py::self)

        DEF_PICKLE(TransRecord);

    py::enum_<TransRecord::DIRECT>(m, "DIRECT")
      .value("BUY", TransRecord::BUY)
      .value("SELL", TransRecord::SELL)
      .value("AUCTION", TransRecord::AUCTION);
}
