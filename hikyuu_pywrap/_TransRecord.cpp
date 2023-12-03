/*
 * _KRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/TransRecord_serialization.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "pybind_utils.h"
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TransRecord() {
    class_<TransRecord>("TransRecord", init<>())
      .def(init<const Datetime&, price_t, price_t, TransRecord::DIRECT>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .def_readwrite("date", &TransRecord::datetime, "时间")
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

    class_<TransList>("TransList")
      .def(vector_indexing_suite<TransList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TransList>())
#endif
      ;
}
