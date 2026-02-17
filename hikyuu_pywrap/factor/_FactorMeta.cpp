/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/factor/FactorMeta.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_FactorMeta(py::module& m) {
    py::class_<FactorMeta>(m, "FactorMeta", "因子元数据")
      .def(py::init<>())
      .def(py::init<const string&, const Indicator&, const KQuery::KType&, const string&,
                    const string&>(),
           py::arg("name"), py::arg("formula"), py::arg("ktype") = KQuery::DAY,
           py::arg("brief") = "", py::arg("details") = "")

      .def("__str__", &FactorMeta::str)
      .def("__repr__", &FactorMeta::str)

      .def_property_readonly("name", py::overload_cast<>(&FactorMeta::name, py::const_),
                             py::return_value_policy::copy, "因子名称")
      .def_property_readonly("ktype", py::overload_cast<>(&FactorMeta::ktype, py::const_),
                             py::return_value_policy::copy, "因子频率类型")
      .def_property_readonly("create_at", py::overload_cast<>(&FactorMeta::createAt, py::const_),
                             py::return_value_policy::copy, "创建日期")
      .def_property_readonly("update_at", py::overload_cast<>(&FactorMeta::updateAt, py::const_),
                             py::return_value_policy::copy, "更改日期")
      .def_property_readonly("formula", &FactorMeta::formula, "因子公式")
      .def_property("brief", py::overload_cast<>(&FactorMeta::brief, py::const_),
                    py::overload_cast<const string&>(&FactorMeta::brief),
                    py::return_value_policy::copy, "基础说明")
      .def_property("details", py::overload_cast<>(&FactorMeta::details, py::const_),
                    py::overload_cast<const string&>(&FactorMeta::details),
                    py::return_value_policy::copy, "详细说明")

      //   DEF_PICKLE(FactorMeta);
      ;
}
