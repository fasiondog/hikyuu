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
            arg("initcash") = 100000, arg("costfunc") = TC_Zero(),
            arg("name")="SYS"));

    def("TC_TestStub", TC_TestStub);

    def("TC_FixedA", crtFixedATC, (arg("commission") = 0.0018,
            arg("lowest_commission") = 5.0,
            arg("stamptax") = 0.001,
            arg("transferfee") = 0.001,
            arg("lowest_transferfee") = 1.0));

    def("TC_Zero", TC_Zero);
}

