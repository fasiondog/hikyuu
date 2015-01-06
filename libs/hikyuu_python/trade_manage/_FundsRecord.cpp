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
    class_<FundsRecord>("FundsRecord", init<>())
            .def(self_ns::str(self))
            .def_readwrite("cash", &FundsRecord::cash)
            .def_readwrite("market_value", &FundsRecord::market_value)
            .def_readwrite("short_market_value", &FundsRecord::short_market_value)
            .def_readwrite("base_cash", &FundsRecord::base_cash)
            .def_readwrite("base_asset", &FundsRecord::base_asset)
            .def_readwrite("borrow_cash", &FundsRecord::borrow_cash)
            .def_readwrite("borrow_asset", &FundsRecord::borrow_asset)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<FundsRecord>())
#endif
            ;
}


