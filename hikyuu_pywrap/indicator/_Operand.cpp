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

string (Operand::*op_read_name)() const = &Operand::name;
void (Operand::*op_write_name)(const string&) = &Operand::name;

Indicator (Operand::*bracket1)(const Indicator&) = &Operand::operator();
Operand (Operand::*bracket2)(const Operand&) = &Operand::operator();

Operand (*Operand_add2)(const Operand&, price_t) = operator+;
Operand (*Operand_sub2)(const Operand&, price_t) = operator-;
Operand (*Operand_mul2)(const Operand&, price_t) = operator*;
Operand (*Operand_div2)(const Operand&, price_t) = operator/;
Operand (*Operand_eq2)(const Operand&, price_t) = operator==;
Operand (*Operand_ne2)(const Operand&, price_t) = operator!=;
Operand (*Operand_gt2)(const Operand&, price_t) = operator>;
Operand (*Operand_lt2)(const Operand&, price_t) = operator<;
Operand (*Operand_ge2)(const Operand&, price_t) = operator>=;
Operand (*Operand_le2)(const Operand&, price_t) = operator<=;

Operand (*OP_AND1)(const Operand&, const Operand&) = OP_AND;
Operand (*OP_AND2)(const Operand&, price_t) = OP_AND;
Operand (*OP_AND3)(price_t, const Operand&) = OP_AND;

Operand (*OP_OR1)(const Operand&, const Operand&) = OP_OR;
Operand (*OP_OR2)(const Operand&, price_t) = OP_OR;
Operand (*OP_OR3)(price_t, const Operand&) = OP_OR;

void export_Operand() {

    class_<Operand>("Operand", init<>())
        .def(init<const Indicator&>())
        .def(init<const Operand&>())
        .def(init<const Operand&, const Operand&>())
        .add_property("name", op_read_name, op_write_name)
        .def(self_ns::str(self))
        .def("__call__", bracket1)
        .def("__call__", bracket2)
        .def("__add__", &Operand::operator+)
        .def("__add__", Operand_add2)
        .def("__sub__", &Operand::operator-)
        .def("__sub__", Operand_sub2)
        .def("__mul__", &Operand::operator*)
        .def("__mul__", Operand_mul2)
        .def("__div__", &Operand::operator/)
        .def("__div__", Operand_div2)
        .def("__truediv__", &Operand::operator/)
        .def("__truediv__", Operand_div2)
        .def("__eq__", &Operand::operator==)
        .def("__eq__", Operand_eq2)
        .def("__ne__", &Operand::operator!=)
        .def("__ne__", Operand_ne2)
        .def("__gt__", &Operand::operator>)
        .def("__gt__", Operand_gt2)
        .def("__lt__", &Operand::operator<)
        .def("__lt__", Operand_lt2)
        .def("__ge__", &Operand::operator>=)
        .def("__ge__", Operand_ge2)
        .def("__le__", &Operand::operator<=)
        .def("__le__", Operand_le2)
#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(normal_pickle_suite<Operand>())
#endif
        ;

    def("OP_AND", OP_AND1);
    def("OP_AND", OP_AND2);
    def("OP_AND", OP_AND3);

    def("OP_OR", OP_OR1);
    def("OP_OR", OP_OR2);
    def("OP_OR", OP_OR3);
}


