/*
 * _Indicator.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Indicator.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_overloads, get, 1, 2)

Indicator (*indicator_add)(const Indicator&, const Indicator&) = operator+;
Indicator (*indicator_sub)(const Indicator&, const Indicator&) = operator-;
Indicator (*indicator_mul)(const Indicator&, const Indicator&) = operator*;
Indicator (*indicator_div)(const Indicator&, const Indicator&) = operator/;

void export_Indicator() {

    class_<Indicator>("Indicator", init<>())
        .def(init<IndicatorImpPtr>())
        .def(self_ns::str(self))
        .add_property("name", &Indicator::name)
        .add_property("long_name", &Indicator::long_name)
        .add_property("discard", &Indicator::discard)
        .def("size", &Indicator::size)
        .def("empty", & Indicator::empty)
        .def("getResultNumber", &Indicator::getResultNumber)
        .def("get", &Indicator::get, get_overloads())
        .def("getResult", &Indicator::getResult)
        .def("getResultAsPriceList", &Indicator::getResultAsPriceList)
        .def("__len__", &Indicator::size)
        .def("__call__", &Indicator::operator())
#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(normal_pickle_suite<Indicator>())
#endif
        ;

    def("indicator_add", indicator_add);
    def("indicator_sub", indicator_sub);
    def("indicator_mul", indicator_mul);
    def("indicator_div", indicator_div);
}
