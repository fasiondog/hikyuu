/*
 * _Indicator.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Indicator.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_overloads, get, 1, 2)

Indicator (*indicator_add1)(const Indicator&, const Indicator&) = operator+;
Indicator (*indicator_sub1)(const Indicator&, const Indicator&) = operator-;
Indicator (*indicator_mul1)(const Indicator&, const Indicator&) = operator*;
Indicator (*indicator_div1)(const Indicator&, const Indicator&) = operator/;
Indicator (*indicator_eq1)(const Indicator&, const Indicator&) = operator==;
Indicator (*indicator_ne1)(const Indicator&, const Indicator&) = operator!=;
Indicator (*indicator_gt1)(const Indicator&, const Indicator&) = operator>;
Indicator (*indicator_lt1)(const Indicator&, const Indicator&) = operator<;
Indicator (*indicator_ge1)(const Indicator&, const Indicator&) = operator>=;
Indicator (*indicator_le1)(const Indicator&, const Indicator&) = operator<=;
Indicator (*indicator_and1)(const Indicator&, const Indicator&) = operator&;
Indicator (*indicator_or1)(const Indicator&, const Indicator&) = operator|;

Indicator (*indicator_add2)(const Indicator&, price_t) = operator+;
Indicator (*indicator_sub2)(const Indicator&, price_t) = operator-;
Indicator (*indicator_mul2)(const Indicator&, price_t) = operator*;
Indicator (*indicator_div2)(const Indicator&, price_t) = operator/;
Indicator (*indicator_eq2)(const Indicator&, price_t) = operator==;
Indicator (*indicator_ne2)(const Indicator&, price_t) = operator!=;
Indicator (*indicator_gt2)(const Indicator&, price_t) = operator>;
Indicator (*indicator_lt2)(const Indicator&, price_t) = operator<;
Indicator (*indicator_ge2)(const Indicator&, price_t) = operator>=;
Indicator (*indicator_le2)(const Indicator&, price_t) = operator<=;
Indicator (*indicator_and2)(const Indicator&, price_t) = operator&;
Indicator (*indicator_or2)(const Indicator&, price_t) = operator|;


string (Indicator::*ind_read_name)() const = &Indicator::name;
void (Indicator::*ind_write_name)(const string&) = &Indicator::name;

void (Indicator::*setContext_1)(const Stock&, const KQuery&) = &Indicator::setContext;
void (Indicator::*setContext_2)(const KData&) = &Indicator::setContext;

Indicator (Indicator::*ind_call_1)(const Indicator&) = &Indicator::operator();
Indicator (Indicator::*ind_call_2)(const KData&) = &Indicator::operator();
Indicator (Indicator::*ind_call_3)() = &Indicator::operator();

void export_Indicator() {

    class_<Indicator>("Indicator", init<>())
        .def(init<IndicatorImpPtr>())
        .def(self_ns::str(self))
        .add_property("name", ind_read_name, ind_write_name)
        .add_property("long_name", &Indicator::long_name)
        .add_property("discard", &Indicator::discard)
        .def("setDiscard", &Indicator::setDiscard)
        .def("getParam", &Indicator::getParam<boost::any>)
        .def("setParam", &Indicator::setParam<object>)
        .def("size", &Indicator::size)
        .def("empty", &Indicator::empty)
        .def("clone", &Indicator::clone)
        .def("formula", &Indicator::formula)
        .def("getResultNumber", &Indicator::getResultNumber)
        .def("get", &Indicator::get, get_overloads())
        .def("getResult", &Indicator::getResult)
        .def("getResultAsPriceList", &Indicator::getResultAsPriceList)
        .def("setContext", setContext_1)
        .def("setContext", setContext_2)
        .def("getContext", &Indicator::getContext)
        .def("getImp", &Indicator::getImp)
        .def("__len__", &Indicator::size)
        //.def("__call__", &Indicator::operator())
        .def("__call__", ind_call_1)
        .def("__call__", ind_call_2)
        .def("__call__", ind_call_3)
#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(normal_pickle_suite<Indicator>())
#endif
        ;

    def("indicator_add", indicator_add1);
    def("indicator_sub", indicator_sub1);
    def("indicator_mul", indicator_mul1);
    def("indicator_div", indicator_div1);
    def("indicator_eq", indicator_eq1);
    def("indicator_ne", indicator_ne1);
    def("indicator_gt", indicator_gt1);
    def("indicator_lt", indicator_lt1);
    def("indicator_ge", indicator_ge1);
    def("indicator_le", indicator_le1);
    def("indicator_and", indicator_and1);
    def("indicator_or", indicator_or1);

    def("indicator_add", indicator_add2);
    def("indicator_sub", indicator_sub2);
    def("indicator_mul", indicator_mul2);
    def("indicator_div", indicator_div2);
    def("indicator_eq", indicator_eq2);
    def("indicator_ne", indicator_ne2);
    def("indicator_gt", indicator_gt2);
    def("indicator_lt", indicator_lt2);
    def("indicator_ge", indicator_ge2);
    def("indicator_le", indicator_le2);
    def("indicator_and", indicator_and2);
    def("indicator_or", indicator_or2);
}
