/*
 * _MarketInfo.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/MarketInfo_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_MarketInfo() {
    docstring_options doc_options(false, true, false);

    class_<MarketInfo>("MarketInfo", init<>())
            .def(init<const string&, const string&, const string&, const string&, const Datetime&>())
            //.def(self_ns::str(self))
            .def("__str__", &MarketInfo::toString)
            .add_property("market", make_function(&MarketInfo::market, return_value_policy<copy_const_reference>()))
            .add_property("name", make_function(&MarketInfo::name, return_value_policy<copy_const_reference>()))
            .add_property("description", make_function(&MarketInfo::description, return_value_policy<copy_const_reference>()))
            .add_property("code", make_function(&MarketInfo::code, return_value_policy<copy_const_reference>()))
            .add_property("lastDate", &MarketInfo::lastDate)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<MarketInfo>())
#endif
            ;
}


