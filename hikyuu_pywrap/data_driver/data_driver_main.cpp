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

void export_data_driver_main() {
    export_BaseInfoDriver();
    export_BlockInfoDriver();
    export_KDataDriver();
    export_DataDriverFactory();
}
