/*
 * _DataDriverFactory.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/data_driver/DataDriverFactory.h>

using namespace hku;
using namespace boost::python;

void export_DataDriverFactory() {

    class_<DataDriverFactory>("DataDriverFactory", no_init)
        .def("getBaseInfoDriver", &DataDriverFactory::getBaseInfoDriver)
        .def("regBaseInfoDriver", &DataDriverFactory::regBaseInfoDriver)
        .def("removeBaseInfoDriver", &DataDriverFactory::removeBaseInfoDriver)
        .def("getKDataDriver", &DataDriverFactory::getKDataDriver)
        .def("regKDataDriver", &DataDriverFactory::regKDataDriver)
        .def("removeKDataDriver", &DataDriverFactory::removeKDataDriver)
        .def("getBlockDriver", &DataDriverFactory::getBlockDriver)
        .def("regBlockDriver", &DataDriverFactory::regBlockDriver)
        .def("removeBlockDriver", &DataDriverFactory::removeBlockDriver)
        .staticmethod("getBaseInfoDriver")
        .staticmethod("regBaseInfoDriver")
        .staticmethod("removeBaseInfoDriver")
        .staticmethod("getKDataDriver")
        .staticmethod("getBlockDriver")
        .staticmethod("regBlockDriver")
        .staticmethod("removeBlockDriver")
        ;

}


