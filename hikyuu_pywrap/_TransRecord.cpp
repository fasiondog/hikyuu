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

void export_TransRecord() {
    class_<TransRecord>("TransRecord", init<>())
      .def(init<const Datetime&, price_t, price_t, TransRecord::DIRECT>())
      .def(self_ns::str(self))
      .def_readwrite("datetime", &TransRecord::datetime, "时间")
      .def_readwrite("price", &TransRecord::price, "价格")
      .def_readwrite("vol", &TransRecord::vol, "成交量")
      .def_readwrite("direct", &TransRecord::direct, "买卖盘性质, 参见: TransRecord.DIRECT")
      .def(self == self)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TransRecord>())
#endif
      ;

    enum_<TransRecord::DIRECT>("DIRECT")
      .value("BUY", TransRecord::BUY)
      .value("SELL", TransRecord::SELL)
      .value("AUCTION", TransRecord::AUCTION);

    TransList::const_reference (TransList::*TransList_at)(TransList::size_type) const =
      &TransList::at;
    void (TransList::*append)(const TransRecord&) = &TransList::push_back;
    class_<TransList>("TransList")
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .def("__iter__", iterator<TransList>())
      .def("size", &TransList::size)
      .def("__len__", &TransList::size)
      .def("get", TransList_at, return_value_policy<copy_const_reference>())
      .def("append", append)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TransList>())
#endif
      ;
}
