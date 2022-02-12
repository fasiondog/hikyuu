/*
 * indicator_main.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <boost/python.hpp>

using namespace boost::python;

void export_Indicator();
void export_IndicatorImp();
void export_IndParam();
void export_Indicator_build_in();

void export_indicator_main() {
    export_Indicator();
    export_IndicatorImp();
    export_IndParam();
    export_Indicator_build_in();
}
