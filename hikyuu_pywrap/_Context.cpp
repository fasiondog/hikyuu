/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/Context.h>

using namespace boost::python;
using namespace hku;

void export_context() {
    def("set_current_context", setGlobalContext);
    def("get_current_context", getGlobalContextKData);
}


