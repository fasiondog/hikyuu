/*
 * _FundsRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/FundsRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_FundsRecord() {
    class_<FundsRecord>("FundsRecord", "当前资产情况记录", init<>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def_readwrite("cash", &FundsRecord::cash, "当前现金（float）")
      .def_readwrite("market_value", &FundsRecord::market_value, "当前多头市值（float）")
      .def_readwrite("base_cash", &FundsRecord::base_cash, "当前投入本金（float）")
      .def_readwrite("base_asset", &FundsRecord::base_asset, "当前投入的资产价值（float）")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<FundsRecord>())
#endif
      ;
}
