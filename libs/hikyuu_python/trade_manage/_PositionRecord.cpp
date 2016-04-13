/*
 * _PositionRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/PositionRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_PositionRecord() {
    class_<PositionRecord>("PositionRecord", init<>())
            .def(init<const Stock&, const Datetime&, const Datetime&, size_t,
                    price_t, price_t, size_t, price_t, price_t, price_t, price_t>())
            //.def(self_ns::str(self))
            .def("__str__", &PositionRecord::toString)
            .def_readwrite("stock", &PositionRecord::stock)
            .def_readwrite("takeDatetime", &PositionRecord::takeDatetime)
            .def_readwrite("cleanDatetime", &PositionRecord::cleanDatetime)
            .def_readwrite("number", &PositionRecord::number)
            .def_readwrite("stoploss", &PositionRecord::stoploss)
            .def_readwrite("goalPrice", &PositionRecord::goalPrice)
            .def_readwrite("totalNumber", &PositionRecord::totalNumber)
            .def_readwrite("buyMoney", &PositionRecord::buyMoney)
            .def_readwrite("totalCost", &PositionRecord::totalCost)
            .def_readwrite("totalRisk", &PositionRecord::totalRisk)
            .def_readwrite("sellMoney", &PositionRecord::sellMoney)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<PositionRecord>())
#endif
            ;

    PositionRecordList::const_reference (PositionRecordList::*PositionRecordList_at)(PositionRecordList::size_type) const = &PositionRecordList::at;
    class_<PositionRecordList>("PositionRecordList")
            .def("__iter__", iterator<PositionRecordList>())
            .def("size", &PositionRecordList::size)
            .def("__len__", &PositionRecordList::size)
            .def("get", PositionRecordList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<PositionRecordList>())
#endif
            ;
}


