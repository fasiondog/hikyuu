/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-28
 *     Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/strategy/AccountTradeManager.h>

using namespace boost::python;
using namespace hku;

void export_AccountTradeManger() {
    def("crtAccountTM", crtAccountTM);
}