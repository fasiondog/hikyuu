/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/plugin/factor.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Factor(py::module& m);
void export_FactorSet(py::module& m);

void export_factor_main(py::module& m) {
    export_Factor(m);
    export_FactorSet(m);

    m.def("get_all_factors", &getAllFactors, "获取所有因子元数据");

    m.def("update_all_factors_values", &updateAllFactorsValues, py::arg("ktype") = KQuery::DAY,
          "更新所有因子值");

    m.def("save_factor_set", &saveFactorSet, py::arg("set"), "保存因子集");
    m.def("get_factor_set", &getFactorSet, py::arg("name"), py::arg("ktype") = KQuery::DAY,
          "获取因子集");
    m.def("remove_factor_set", &removeFactorSet, py::arg("name"), py::arg("ktype"), "删除因子集");
    m.def("get_all_factor_sets", &getAllFactorSets, "获取所有因子集");
}
