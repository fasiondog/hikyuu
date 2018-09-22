/*
 * ioredirect.cpp
 *
 *  Created on: 2018年8月27日
 *      Author: fasiondog
 */

#include "ioredirect.h"

using namespace boost::python;

void export_io_redirect() {
    boost::python::docstring_options doc_options(false);
    
    class_<OstreamRedirect>("OstreamRedirect", init<bool,bool>((arg("stdout")=true, arg("stderr")=true)))
        .def("__enter__", &OstreamRedirect::enter)
        .def("__exit__", &OstreamRedirect::exit)
        .def("open", &OstreamRedirect::enter)
        .def("close", &OstreamRedirect::exit)
        ;
}
