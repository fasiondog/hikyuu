/*
 * _AmaInstance.cpp
 *
 *  Created on: 2015年3月14日
 *      Author: fasiondog
 */


#include <boost/python.hpp>
#include <hikyuu/trade_instance/ama_sys/AmaInstance.h>

using namespace boost::python;
using namespace hku;

void export_AmaInstance() {
    def("AmaSpecial", AmaSpecial);
}
