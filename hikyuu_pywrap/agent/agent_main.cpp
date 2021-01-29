/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <boost/python.hpp>

using namespace boost::python;

void export_SpotAgent();

void export_agent_main() {
    export_SpotAgent();
}
