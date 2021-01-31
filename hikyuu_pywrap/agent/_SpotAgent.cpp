/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/agent/SpotAgent.h>

using namespace boost::python;
using namespace hku;

void export_SpotAgent() {
    def("start_spot_agent", start_spot_agent, (arg("print") = false));
    def("stop_spot_agent", stop_spot_agent);
}
