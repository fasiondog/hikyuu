/*
 * instance_main.cpp
 *
 *  Created on: 2015年3月14日
 *      Author: fasiondog
 */

#include <boost/python.hpp>

using namespace boost::python;

void export_AmaInstance();

void export_instance_main() {
    export_AmaInstance();
}
