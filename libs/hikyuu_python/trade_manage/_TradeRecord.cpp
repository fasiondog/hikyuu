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
            .value("BUSINESS_INIT", BUSINESS_INIT)
            .value("BUSINESS_BUY", BUSINESS_BUY)
            .value("BUSINESS_SELL", BUSINESS_SELL)
            .value("BUSINESS_GIFT", BUSINESS_GIFT)
            .value("BUSINESS_BONUS", BUSINESS_BONUS)
            .value("BUSINESS_CHECKIN", BUSINESS_CHECKIN)
            .value("BUSINESS_CHECKOUT", BUSINESS_CHECKOUT)
            .value("BUSINESS_CHECKIN_STOCK", BUSINESS_CHECKIN_STOCK)
            .value("BUSINESS_CHECKOUT_STOCK", BUSINESS_CHECKOUT_STOCK)
            .value("BUSINESS_BORROW_CASH", BUSINESS_BORROW_CASH)
            .value("BUSINESS_RETURN_CASH", BUSINESS_RETURN_CASH)
            .value("BUSINESS_BORROW_STOCK", BUSINESS_BORROW_STOCK)
            .value("BUSINESS_RETURN_STOCK", BUSINESS_RETURN_STOCK)
            .value("INVALID_BUSINESS", INVALID_BUSINESS)
            ;

    def("getBusinessName", getBusinessName);

    class_<TradeRecord>("TradeRecord", init<>())
            .def(init<const Stock&, const Datetime&, BUSINESS, price_t,
                    price_t, price_t, size_t, const CostRecord&,
                    price_t, price_t, SystemPart>())
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
            .def_readwrite("part", &TradeRecord::from) //python中不能用from关键字
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TradeRecord>())
#endif
            ;

    TradeRecordList::const_reference (TradeRecordList::*TradeRecordList_at)(TradeRecordList::size_type) const = &TradeRecordList::at;
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


