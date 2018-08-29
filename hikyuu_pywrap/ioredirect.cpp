/*
 * ioredirect.cpp
 *
 *  Created on: 2018年8月27日
 *      Author: fasiondog
 */

#include "ioredirect.h"

using namespace boost::python;

void export_io_redirect() {
    class_<ostream_redirect>("ostream_redirect", init<>())
           ;
}





