/*
 * _Stock.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/Stock_serialization.h>
#include <hikyuu/KData.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getCount_overloads, getCount, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getIndex_overloads, getIndex, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getRecord_overloads, getKRecord, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getRecordByDate_overloads, getKRecordByDate, 1, 2)

StockWeightList (Stock::*getWeight1)() const = &Stock::getWeight;
StockWeightList (Stock::*getWeight2)(const Datetime&, const Datetime&) const = &Stock::getWeight;

DatetimeList (Stock::*getDatetimeList1)(size_t, size_t, KQuery::KType) const = &Stock::getDatetimeList;
DatetimeList (Stock::*getDatetimeList2)(const KQuery&) const = &Stock::getDatetimeList;


void export_Stock() {
    docstring_options doc_options(false);

    class_<Stock>("Stock", init<>())
            .def(init<const string&, const string&, const string&>())
            //.def(self_ns::str(self))
            .def("__str__", &Stock::toString)
            .add_property("id", &Stock::id)
            .add_property("market", make_function(&Stock::market, return_value_policy<copy_const_reference>()))
            .add_property("code", make_function(&Stock::code, return_value_policy<copy_const_reference>()))
            .add_property("market_code", make_function(&Stock::market_code, return_value_policy<copy_const_reference>()))
            .add_property("name", make_function(&Stock::name, return_value_policy<copy_const_reference>()))
            .add_property("type", &Stock::type)
            .add_property("valid", &Stock::valid)
            .add_property("startDatetime", &Stock::startDatetime)
            .add_property("lastDatetime", &Stock::lastDatetime)
            .add_property("tick", &Stock::tick)
            .add_property("tickValue", &Stock::tickValue)
            .add_property("unit", &Stock::unit)
            .add_property("precision", &Stock::precision)
            .add_property("atom", &Stock::atom)
            .add_property("minTradeNumber", &Stock::minTradeNumber)
            .add_property("maxTradeNumber", &Stock::maxTradeNumber)

            .def("isNull", &Stock::isNull)
            .def("isBuffer", &Stock::isBuffer)
            .def("getKData", &Stock::getKData)
            .def("getCount", &Stock::getCount, getCount_overloads())
            .def("getMarketValue", &Stock::getMarketValue)
            .def("getKRecord", &Stock::getKRecord, getRecord_overloads())
            .def("getKRecordByDate", &Stock::getKRecordByDate, getRecordByDate_overloads())
            .def("getKRecordList", &Stock::getKRecordList)
            .def("getDatetimeList", getDatetimeList1)
            .def("getDatetimeList", getDatetimeList2)
            .def("realtimeUpdate", &Stock::realtimeUpdate)
            .def("getWeight", getWeight1)
            .def("getWeight", getWeight2)
            .def("loadKDataToBuffer", &Stock::loadKDataToBuffer)
            .def("releaseKDataBuffer", &Stock::releaseKDataBuffer)

            .def("__eq__", &Stock::operator==)
            .def("__ne__", &Stock::operator!=)

#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<Stock>())
#endif
            ;
}
