/*
 * _LoanRecord.cpp
 *
 *  Created on: 2013-5-24
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/LoanRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_LoanRecord() {
    class_<LoanRecord>("LoanRecord", "借款记录（融资记录）", init<>())
      .def(init<const Datetime&, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .def_readwrite("datetime", &LoanRecord::datetime, "借款时间")
      .def_readwrite("value", &LoanRecord::value, "借款金额")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<LoanRecord>())
#endif
      ;
}
