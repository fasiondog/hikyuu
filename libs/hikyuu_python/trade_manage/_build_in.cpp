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

TradeCostPtr (*crtFixedATC1)(price_t, price_t, price_t, price_t, price_t) = crtFixedATC;
BOOST_PYTHON_FUNCTION_OVERLOADS(crtFixedATC1_overloads, crtFixedATC, 0, 5);

void export_build_in() {
    def("TestStub_TC", TestStub_TC);

    def("crtZeroTC", crtZeroTC);
    def("crtFixedATC", crtFixedATC1, crtFixedATC1_overloads());
}

