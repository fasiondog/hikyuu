/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include <hikyuu/plugin/KDataToHdf5Importer.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_KDataToHdf5Importer(py::module& m) {
    // py::class_<ImportKDataToHdf5>(m, "Block", "板块类，可视为证券的容器")
    //   .def(py::init<>())
    //   .def(py::init<const string&, const string&>())
    //   .def(py::init<const Block&>())

    //   .def("__str__", to_py_str<Block>)
    //   .def("__repr__", to_py_str<Block>);
}