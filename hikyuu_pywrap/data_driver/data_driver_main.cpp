/*
 * data_driver_main.cpp
 *
 *  Created on: 2017-10-7
 *      Author: fasiondog
 */

#include <boost/python.hpp>

using namespace boost::python;

void export_DataDriverFactory();
void export_KDataDriver();
void export_BaseInfoDriver();
void export_BlockInfoDriver();

BOOST_PYTHON_MODULE(_data_driver) {
    export_BaseInfoDriver();
    export_BlockInfoDriver();
    export_KDataDriver();
    export_DataDriverFactory();
}


