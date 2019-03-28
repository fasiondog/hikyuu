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

void (*setGlobalContext_1)(const Stock&, const KQuery&) = setGlobalContext;
void (*setGlobalContext_2)(const KData&) = setGlobalContext;

void export_context() {
    def("set_current_context", setGlobalContext_1);
    def("set_current_context", setGlobalContext_2);
    def("get_current_context", getGlobalContextKData);
}


