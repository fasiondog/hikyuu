/*
 * _StockTypeInfo.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/StockTypeInfo_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_StockTypeInfo() {
    docstring_options doc_options(false, true, false);

    class_<StockTypeInfo>("StockTypeInfo", init<>())
            .def(init<hku_uint32, const string&, price_t, price_t, int, size_t, size_t>())
            //.def(self_ns::str(self))
            .def("__str__", &StockTypeInfo::toString)
            .add_property("type", &StockTypeInfo::type)
            .add_property("description", make_function(&StockTypeInfo::description, return_value_policy<copy_const_reference>()))
            .add_property("tick", &StockTypeInfo::tick)
            .add_property("tickValue", &StockTypeInfo::tickValue)
            .add_property("unit", &StockTypeInfo::unit)
            .add_property("precision", &StockTypeInfo::precision)
            .add_property("minTradeNumber", &StockTypeInfo::minTradeNumber)
            .add_property("maxTradeNumber", &StockTypeInfo::maxTradeNumber)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<StockTypeInfo>())
#endif
            ;
}
