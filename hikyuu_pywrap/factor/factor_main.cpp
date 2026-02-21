/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/factor/Factor.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Factor(py::module& m);

void export_factor_main(py::module& m) {
    export_Factor(m);
}
