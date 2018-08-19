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
void export_Indicator_build_in();
void export_Operand();

BOOST_PYTHON_MODULE(_indicator) {
    export_Indicator();
    export_IndicatorImp();
    export_Indicator_build_in();
    export_Operand();
}
