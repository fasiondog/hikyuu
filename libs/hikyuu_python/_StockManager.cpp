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
            .def("__len__", &StockManager::size)
            .def("__getitem__", &StockManager::getStock)
            .def("__iter__", iterator<StockManager>())
            ;
}
