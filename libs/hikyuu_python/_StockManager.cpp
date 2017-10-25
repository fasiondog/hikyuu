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

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addTempCsvStock_overloads,
        addTempCsvStock, 3, 8)

BlockList (StockManager::*getBlockList_1)(const string&) = &StockManager::getBlockList;
BlockList (StockManager::*getBlockList_2)() = &StockManager::getBlockList;

void export_StockManager(){
    docstring_options doc_options(false);

    class_<StockManager>("StockManager", no_init)
            .def("instance", &StockManager::instance,
                 return_value_policy<reference_existing_object>())
            .staticmethod("instance")
            .def("init", &StockManager::init)
            .def("setKDataDriver", &StockManager::setKDataDriver)
            .def("tmpdir", &StockManager::tmpdir)
            .def("getBaseInfoDriverParameter", &StockManager::getBaseInfoDriverParameter)
            .def("getBlockDriverParameter", &StockManager::getBlockDriverParameter)
            .def("getKDataDriverParameter", &StockManager::getKDataDriverParameter)
            .def("getPreloadParameter", &StockManager::getPreloadParameter)
            .def("getHikyuuParameter", &StockManager::getHikyuuParameter)
            .def("getAllMarket", &StockManager::getAllMarket)
            .def("getMarketInfo", &StockManager::getMarketInfo)
            .def("getStockTypeInfo", &StockManager::getStockTypeInfo)
            .def("size", &StockManager::size)
            .def("getStock", &StockManager::getStock)
            .def("getBlock", &StockManager::getBlock)
            .def("getBlockList", getBlockList_1)
            .def("getBlockList", getBlockList_2)
            //.def("getTradingCalendar", &StockManager::getTradingCalendar,
            //        getTradingCalendar_overloads())
            .def("getTradingCalendar", &StockManager::getTradingCalendar,
                    (arg("query"), arg("market")="SH"))
            .def("addTempCsvStock", &StockManager::addTempCsvStock,
                    addTempCsvStock_overloads())
            .def("removeTempCsvStock", &StockManager::removeTempCsvStock)

            .def("__len__", &StockManager::size)
            .def("__getitem__", &StockManager::getStock)
            .def("__iter__", iterator<const StockManager>())
            ;
}
