/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-24
 *      Author: fasiondog
 */

#include <hikyuu/factor/FactorSet.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_FactorSet(py::module& m) {
    py::class_<FactorSet>(m, "FactorSet", "因子元数据")
      .def(py::init<>())
      .def(py::init<const string&, const KQuery::KType&, const Block&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY, py::arg("block") = Block())

      .def("__str__", &FactorSet::str)
      .def("__repr__", &FactorSet::str)

      .def_property_readonly("name", py::overload_cast<>(&FactorSet::name, py::const_),
                             py::return_value_policy::copy, "因子名称")
      .def_property_readonly("ktype", py::overload_cast<>(&FactorSet::ktype, py::const_),
                             py::return_value_policy::copy, "因子频率类型")

      .def_property("block", py::overload_cast<>(&FactorSet::block, py::const_),
                    py::overload_cast<const Block&>(&FactorSet::block),
                    py::return_value_policy::copy)

      .def("is_null", &FactorSet::isNull, R"(is_null(self)
        
    是否为null值)")

      .def("empty", &FactorSet::empty, R"(empty(self)
    
    是否为空)")

      .def("clear", &FactorSet::clear, R"(clear(self))

    清空因子元数据)")

      .def("have", &FactorSet::have)
      .def("remove", &FactorSet::remove)
      .def("add", py::overload_cast<const Factor&>(&FactorSet::add))

      .def("get_factors", &FactorSet::getAllFactors, py::return_value_policy::copy)

      .def("get_all_values", &FactorSet::getAllValues, py::arg("query"), py::arg("align") = false,
           py::arg("fill_null") = false, py::arg("tovalue") = true)

      .def(
        "get_values",
        [](FactorSet& self, const py::object& stks, const KQuery& query, bool align = false,
           bool fill_null = false, bool tovalue = true, bool check = false) {
            return self.getValues(get_stock_list_from_python(stks), query, align, fill_null,
                                  tovalue, check);
        },
        py::arg("stocks"), py::arg("query"), py::arg("align") = false, py::arg("fill_null") = false,
        py::arg("tovalue") = true, py::arg("check") = false)

      .def("__getitem__", py::overload_cast<const string&>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__getitem__", py::overload_cast<size_t>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__len__", &FactorSet::size, "包含的因子数量")
      .def(
        "__iter__",
        [](const FactorSet& self) { return py::make_iterator(self.begin(), self.end()); },
        py::keep_alive<0, 1>())

      ;
}