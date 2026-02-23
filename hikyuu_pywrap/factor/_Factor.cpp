/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/factor/Factor.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Factor(py::module& m) {
    py::class_<Factor>(m, "Factor", "因子元数据")
      .def(py::init<>())
      .def(py::init<const string&, const KQuery::KType&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY)
      .def(py::init<const string&, const Indicator&, const KQuery::KType&, const string&,
                    const string&, bool, const Datetime&, const Block&>(),
           py::arg("name"), py::arg("formula"), py::arg("ktype") = KQuery::DAY,
           py::arg("brief") = "", py::arg("details") = "", py::arg("need_persist") = false,
           py::arg("start_date") = Datetime::min(), py::arg("block") = Block())

      .def("__str__", &Factor::str)
      .def("__repr__", &Factor::str)

      .def_property_readonly("name", py::overload_cast<>(&Factor::name, py::const_),
                             py::return_value_policy::copy, "因子名称")
      .def_property_readonly("ktype", py::overload_cast<>(&Factor::ktype, py::const_),
                             py::return_value_policy::copy, "因子频率类型")
      .def_property_readonly("create_at", py::overload_cast<>(&Factor::createAt, py::const_),
                             py::return_value_policy::copy, "创建日期")
      .def_property_readonly("update_at", py::overload_cast<>(&Factor::updateAt, py::const_),
                             py::return_value_policy::copy, "更改日期")
      .def_property_readonly("formula", &Factor::formula, "因子公式")
      .def_property_readonly("start_date", py::overload_cast<>(&Factor::startDate, py::const_),
                             "数据存储起始日期")
      .def_property_readonly("block", py::overload_cast<>(&Factor::block, py::const_),
                             py::return_value_policy::copy, "证券集合")
      .def_property("brief", py::overload_cast<>(&Factor::brief, py::const_),
                    py::overload_cast<const string&>(&Factor::brief), py::return_value_policy::copy,
                    "基础说明")
      .def_property("details", py::overload_cast<>(&Factor::details, py::const_),
                    py::overload_cast<const string&>(&Factor::details),
                    py::return_value_policy::copy, "详细说明")
      .def_property("need_persist", py::overload_cast<>(&Factor::needPersist, py::const_),
                    py::overload_cast<bool>(&Factor::needPersist), "是否持久化")

      .def("save", &Factor::save, "保存因子元数据")
      .def("remove", &Factor::remove, "删除因子元数据")
      .def("get_all_values", &Factor::getAllValues)
      .def(
        "get_values",
        [](Factor& self, const py::object& stks, const KQuery& query, bool check = false) {
            return self.getValues(get_stock_list_from_python(stks), query);
        },
        py::arg("stocks"), py::arg("query"), py::arg("check") = false)

      .def(py::hash(py::self))

      ;
}
