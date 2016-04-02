/*
 * _StockManager.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/StockManager.h>

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getTradingCalendar_overloads,
        getTradingCalendar, 1, 2)

BlockList (StockManager::*getBlockList_1)(const string&) = &StockManager::getBlockList;
BlockList (StockManager::*getBlockList_2)() = &StockManager::getBlockList;

void export_StockManager(){
    class_<StockManager>("StockManager", no_init)
            .def("instance", &StockManager::instance,
                 return_value_policy<reference_existing_object>())
            .staticmethod("instance")
            .def("tmpdir", &StockManager::tmpdir)
            .def("getMarketInfo", &StockManager::getMarketInfo)
            .def("getStockTypeInfo", &StockManager::getStockTypeInfo)
            .def("size", &StockManager::size)
            .def("getStock", &StockManager::getStock)
            .def("getBlock", &StockManager::getBlock)
            .def("getBlockList", getBlockList_1)
            .def("getBlockList", getBlockList_2)
            .def("getTradingCalendar", &StockManager::getTradingCalendar,
                    getTradingCalendar_overloads())
            //.def("getTradingCalendar", &StockManager::getTradingCalendar,
            //        (arg("market")="SH", arg("start")=Datetime::minDatetime(),
            //        arg=("end")=Datetime::maxDatetime()))

            .def("__len__", &StockManager::size)
            .def("__getitem__", &StockManager::getStock)
            //.def("__iter__", iterator<StockManager>())
            .def("__iter__", iterator<const StockManager>())
            ;
}
