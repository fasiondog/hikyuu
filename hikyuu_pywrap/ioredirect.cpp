/*
 * ioredirect.cpp
 *
 *  Created on: 2018年8月27日
 *      Author: fasiondog
 */

#include <pybind11/iostream.h>
#include "ioredirect.h"

namespace hku {

pybind11::detail::OstreamRedirect OStreamToPython::ms_io_redirect(true, true);
bool OStreamToPython::ms_opened{false};

void open_ostream_to_python() {
    if (!OStreamToPython::ms_opened) {
        OStreamToPython::ms_io_redirect.enter();
        OStreamToPython::ms_opened = true;
    }
}

void close_ostream_to_python() {
    if (OStreamToPython::ms_opened) {
        OStreamToPython::ms_io_redirect.exit();
        OStreamToPython::ms_opened = false;
    }
}

}  // namespace hku

namespace py = pybind11;
using namespace hku;

void export_io_redirect(py::module &m) {
    m.def("open_ostream_to_python", open_ostream_to_python);
    m.def("close_ostream_to_python", close_ostream_to_python);
}
