/*
 * _Operand.cpp
 *
 *  Created on: 2015年3月31日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Operand.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

string (Operand::*read_name)() const = &Operand::name;
void (Operand::*write_name)(const string&) = &Operand::name;

Indicator (Operand::*bracket1)(const Indicator&) = &Operand::operator();
Operand (Operand::*bracket2)(const Operand&) = &Operand::operator();

void export_Operand() {

    class_<Operand>("Operand", init<>())
        .def(init<const Indicator&>())
        .def(init<const Operand&>())
        .def(init<const Operand&, const Operand&>())
        .add_property("name", read_name, write_name)
        .def(self_ns::str(self))
        .def("__call__", bracket1)
        .def("__call__", bracket2)
        .def("__add__", &Operand::operator+)
        .def("__sub__", &Operand::operator-)
        .def("__mul__", &Operand::operator*)
        .def("__div__", &Operand::operator/)
        .def("__truediv__", &Operand::operator/)
#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(normal_pickle_suite<Operand>())
#endif
        ;
}


