/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/TradeRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_TradeRecord() {
    enum_<BUSINESS>("BUSINESS")
      .value("INIT", BUSINESS_INIT)
      .value("BUY", BUSINESS_BUY)
      .value("SELL", BUSINESS_SELL)
      .value("GIFT", BUSINESS_GIFT)
      .value("BONUS", BUSINESS_BONUS)
      .value("CHECKIN", BUSINESS_CHECKIN)
      .value("CHECKOUT", BUSINESS_CHECKOUT)
      .value("CHECKIN_STOCK", BUSINESS_CHECKIN_STOCK)
      .value("CHECKOUT_STOCK", BUSINESS_CHECKOUT_STOCK)
      .value("BORROW_CASH", BUSINESS_BORROW_CASH)
      .value("RETURN_CASH", BUSINESS_RETURN_CASH)
      .value("BORROW_STOCK", BUSINESS_BORROW_STOCK)
      .value("RETURN_STOCK", BUSINESS_RETURN_STOCK)
      .value("INVALID", BUSINESS_INVALID);

    def("getBusinessName", getBusinessName);

    class_<TradeRecord>("TradeRecord", init<>())
      .def(init<const Stock&, const Datetime&, BUSINESS, price_t, price_t, price_t, double,
                const CostRecord&, price_t, price_t, SystemPart>())
      //.def(self_ns::str(self))
      .def("__str__", &TradeRecord::toString)
      .def_readwrite("stock", &TradeRecord::stock)
      .def_readwrite("datetime", &TradeRecord::datetime)
      .def_readwrite("business", &TradeRecord::business)
      .def_readwrite("planPrice", &TradeRecord::planPrice)
      .def_readwrite("realPrice", &TradeRecord::realPrice)
      .def_readwrite("goalPrice", &TradeRecord::goalPrice)
      .def_readwrite("number", &TradeRecord::number)
      .def_readwrite("cost", &TradeRecord::cost)
      .def_readwrite("stoploss", &TradeRecord::stoploss)
      .def_readwrite("cash", &TradeRecord::cash)
      .def_readwrite("part", &TradeRecord::from)  // python中不能用from关键字
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TradeRecord>())
#endif
      ;

    TradeRecordList::const_reference (TradeRecordList::*TradeRecordList_at)(
      TradeRecordList::size_type) const = &TradeRecordList::at;
    class_<TradeRecordList>("TradeRecordList")
      .def("__iter__", iterator<TradeRecordList>())
      .def("size", &TradeRecordList::size)
      .def("__len__", &TradeRecordList::size)
      .def("get", TradeRecordList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TradeRecordList>())
#endif
      ;
}
