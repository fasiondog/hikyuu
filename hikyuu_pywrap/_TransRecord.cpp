/*
 * _KRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TransRecord_serialization.h>
#include "pybind_utils.h"
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*transrecord_eq)(const TransRecord&, const TransRecord&) = operator==;

void export_TransRecord() {
    class_<TransRecord>("TransRecord", init<>())
      .def(init<const Datetime&, price_t, price_t, TransRecord::DIRECT>())
      .def(self_ns::str(self))
      .def_readwrite("datetime", &TransRecord::datetime)
      .def_readwrite("price", &TransRecord::price)
      .def_readwrite("vol", &TransRecord::vol)
      .def_readwrite("direct", &TransRecord::direct)
      .def("__eq__", transrecord_eq)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TransRecord>())
#endif
      ;

    enum_<TransRecord::DIRECT>("DIRECT")
      .value("BUY", TransRecord::BUY)
      .value("SELL", TransRecord::SELL)
      .value("AUCTION", TransRecord::AUCTION);

    VECTOR_TO_PYTHON_CONVERT(TransRecordList);
}
