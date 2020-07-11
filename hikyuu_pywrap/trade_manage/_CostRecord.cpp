/*
 * _CostRecord.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/CostRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_CostRecord() {
    class_<CostRecord>("CostRecord", "交易成本记录", init<>())
      .def(init<price_t, price_t, price_t, price_t, price_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def_readwrite("commission", &CostRecord::commission, "佣金(float)")
      .def_readwrite("stamptax", &CostRecord::stamptax, "印花税(float)")
      .def_readwrite("transferfee", &CostRecord::transferfee, "过户费(float)")
      .def_readwrite("others", &CostRecord::others, "其它费用(float)")
      .def_readwrite("total", &CostRecord::total,
                     "总成本(float)，= 佣金 + 印花税 + 过户费 + 其它费用")

      .def(self == self)

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<CostRecord>())
#endif
      ;
}
