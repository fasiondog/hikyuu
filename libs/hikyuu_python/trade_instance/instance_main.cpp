/*
 * instance_main.cpp
 *
 *  Created on: 2015年3月14日
 *      Author: fasiondog
 */


#include <boost/python.hpp>

using namespace boost::python;

void export_AmaInstance();

BOOST_PYTHON_MODULE(_trade_instance) {
    export_AmaInstance();
}


