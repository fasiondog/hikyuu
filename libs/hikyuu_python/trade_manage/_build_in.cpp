/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/build_in.h>

using namespace boost::python;
using namespace hku;

void export_build_in() {
    def("crtTM", crtTM, (arg("datetime") = Datetime(199001010000LL),
            arg("initcash") = 100000, arg("costfunc") = Zero_TC(),
            arg("name")="SYS"));

    def("TestStub_TC", TestStub_TC);

    def("FixedA_TC", crtFixedATC, (arg("commission") = 0.0018,
            arg("lowest_commission") = 5.0,
            arg("stamptax") = 0.001,
            arg("transferfee") = 0.001,
            arg("lowest_transferfee") = 1.0));

    def("Zero_TC", Zero_TC);
}

