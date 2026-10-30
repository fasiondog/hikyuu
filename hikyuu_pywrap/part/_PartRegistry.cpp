/*
 * _PartRegistry.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <hikyuu/part/PartRegistry.h>

#include <fmt/format.h>

#include "part_convert.h"

namespace py = pybind11;
using namespace hku;

void export_python_part(py::module& part);

void export_part_main(py::module& m) {
    py::module part = m.def_submodule("part", "Part registry: meta info and creation by name");

    py::enum_<PartType>(part, "PartType", "Part category")
      .value("IND", PartType::IND, "Indicator")
      .value("SG", PartType::SG, "Signal")
      .value("MM", PartType::MM, "Money manager")
      .value("ST", PartType::ST, "Stoploss")
      .value("CN", PartType::CN, "Condition")
      .value("EV", PartType::EV, "Environment")
      .value("PG", PartType::PG, "Profit goal")
      .value("SP", PartType::SP, "Slippage")
      .value("PF", PartType::PF, "Portfolio")
      .value("SE", PartType::SE, "Selector")
      .value("AF", PartType::AF, "Allocate funds")
      .value("MF", PartType::MF, "Multi factor")
      .value("SYS", PartType::SYS, "System")
      .value("TM", PartType::TM, "Trade manager")
      .value("TC", PartType::TC, "Trade cost")
      .export_values();

    py::enum_<PartMeta::Source>(part, "Source", "Where a part comes from")
      .value("BUILTIN", PartMeta::Source::BUILTIN, "Built into the core library")
      .value("PLUGIN", PartMeta::Source::PLUGIN, "Loaded from a plugin")
      .value("PYTHON", PartMeta::Source::PYTHON, "Defined in Python")
      .export_values();

    py::class_<ParamMeta>(part, "ParamMeta", "Meta info of a single parameter")
      .def(py::init<>())
      .def_readwrite("name", &ParamMeta::name, "Parameter name")
      .def_readwrite("type", &ParamMeta::type, "Parameter type")
      .def_readwrite("default_value", &ParamMeta::default_value, "Default value as string")
      .def_readwrite("desc", &ParamMeta::desc, "Description")
      .def_readwrite("required", &ParamMeta::required, "Whether the parameter is required")
      .def_readwrite("is_ind_param", &ParamMeta::is_ind_param,
                     "Whether the IndParam form is supported")
      .def_readwrite("is_part_param", &ParamMeta::is_part_param,
                     "Whether it is carried by the part param map")
      .def_readwrite("is_list", &ParamMeta::is_list, "Whether it is a part-typed list");

    py::class_<PartMeta>(part, "PartMeta", "Meta info of a registered part")
      .def(py::init<>())
      .def_readwrite("type", &PartMeta::type, "Part category")
      .def_readwrite("name", &PartMeta::name, "C++ name")
      .def_readwrite("full_name", &PartMeta::full_name, "Registry key")
      .def_readwrite("author", &PartMeta::author, "Author")
      .def_readwrite("version", &PartMeta::version, "Version")
      .def_readwrite("doc", &PartMeta::doc, "Documentation")
      .def_readwrite("label", &PartMeta::label, "Labels")
      .def_readwrite("category", &PartMeta::category, "Category")
      .def_readwrite("params", &PartMeta::params, "Parameters in signature order")
      .def_readwrite("result_num", &PartMeta::result_num, "Indicator only: result count")
      .def_readwrite("support_dynamic_param", &PartMeta::support_dynamic_param,
                     "Indicator only: dynamic param support")
      .def_readwrite("support_increment", &PartMeta::support_increment,
                     "Indicator only: incremental calculation support")
      .def_readwrite("source", &PartMeta::source, "Where the part comes from")
      .def_readwrite("support_batch_reuse", &PartMeta::support_batch_reuse,
                     "Whether the batch reuse fast path is safe")
      .def_readwrite("module_hint", &PartMeta::module_hint, "Plugin/module name")
      .def_property_readonly(
        "type_name", [](const PartMeta& m) { return partTypeName(m.type); }, "Category string");

    py::class_<PartRegistry>(part, "PartRegistry", "The process wide part registry")
      .def_static("instance", &PartRegistry::instance, py::return_value_policy::reference,
                  "Get the singleton")
      .def("has", &PartRegistry::has, py::arg("full_name"),
           "Whether a part with the given full name is registered")
      .def("get_meta", &PartRegistry::getMeta, py::arg("full_name"), py::return_value_policy::copy,
           "Get the meta info, throws when not registered")
      .def(
        "try_get_meta",
        [](const PartRegistry& reg, const std::string& full_name) -> std::optional<PartMeta> {
            PartMeta out;
            if (reg.tryGetMeta(full_name, out)) {
                return out;
            }
            return std::nullopt;
        },
        py::arg("full_name"), "Get the meta info, returns None when not registered")
      .def("get_name_list", py::overload_cast<>(&PartRegistry::getNameList, py::const_),
           "All full names, sorted")
      .def("get_name_list", py::overload_cast<PartType>(&PartRegistry::getNameList, py::const_),
           py::arg("type"), "Full names of the given category")
      .def("get_meta_list", &PartRegistry::getMetaList, py::arg("type"),
           "Meta infos of the given category")
      .def("search", &PartRegistry::search, py::arg("keyword"), py::arg("type") = std::nullopt,
           py::arg("label") = "", "Keyword search over the meta infos")
      .def(
        "create",
        [](const PartRegistry& reg, const std::string& full_name) {
            return pywrap::partToPython(reg.create(full_name));
        },
        py::arg("full_name"), "Create a part with all params at their defaults")
      .def(
        "create",
        [](const PartRegistry& reg, const std::string& full_name, const Parameter& scalars) {
            return pywrap::partToPython(reg.create(full_name, scalars));
        },
        py::arg("full_name"), py::arg("scalars"), "Create a part overriding scalar params")
      .def(
        "create_from_string",
        [](const PartRegistry& reg, const std::string& expr) {
            return pywrap::partToPython(reg.createFromString(expr));
        },
        py::arg("expr"), "Create a part from an expression like \"MA(n=10)\"")
      .def("to_json", &PartRegistry::toJson, py::arg("type") = std::nullopt,
           "JSON dump of the meta infos")
      .def("size", &PartRegistry::size, "Number of registered parts")
      .def("unregister", &PartRegistry::unregister, py::arg("full_name"),
           "Unregister a part, returns whether it was found")
      .def("__repr__", [](const PartRegistry& reg) {
          return fmt::format("<PartRegistry size={}>", reg.size());
      });

    export_python_part(part);
}
