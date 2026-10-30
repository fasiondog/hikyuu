/*
 * _PythonPart.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include <algorithm>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <hikyuu/part/PartRegistry.h>
#include <hikyuu/utilities/Log.h>

#include "part_convert.h"

namespace py = pybind11;
using namespace hku;

namespace {

struct PythonPartHolder {
    std::string full_name;
    py::object factory;
};

/*
 * Intentionally leaked: the container holds py::object values and must never be destroyed
 * after the interpreter has been finalized. release_python_parts() clears it while the
 * interpreter is still alive.
 */
std::vector<PythonPartHolder>* g_python_parts = nullptr;

std::vector<PythonPartHolder>& holders() {
    if (g_python_parts == nullptr) {
        g_python_parts = new std::vector<PythonPartHolder>();
    }
    return *g_python_parts;
}

py::object findFactory(const std::string& full_name) {
    for (const auto& holder : holders()) {
        if (holder.full_name == full_name) {
            return holder.factory;
        }
    }
    return py::none();
}

bool interpreterShuttingDown() {
#if PY_VERSION_HEX >= 0x030D0000
    return Py_IsFinalizing() != 0;
#else
    return Py_IsInitialized() == 0;
#endif
}

}  // namespace

void registerPythonPart(const std::string& full_name, const PartMeta& meta, py::object factory,
                        bool overwrite) {
    HKU_CHECK_THROW(!full_name.empty(), std::invalid_argument, "Empty part full_name: {}",
                    full_name);
    HKU_CHECK_THROW(!factory.is_none() && PyCallable_Check(factory.ptr()) != 0,
                    std::invalid_argument, "The factory of part \"{}\" is not callable", full_name);

    PartMeta part_meta(meta);
    part_meta.full_name = full_name;
    part_meta.source = PartMeta::Source::PYTHON;
    part_meta.support_batch_reuse = false;

    auto& reg = PartRegistry::instance();
    HKU_CHECK_THROW(overwrite || !reg.has(full_name), std::invalid_argument,
                    "Part \"{}\" is already registered (builtin or plugin); pass overwrite=True "
                    "to replace it",
                    full_name);

    reg.unregister(full_name);
    auto& list = holders();
    list.erase(std::remove_if(list.begin(), list.end(),
                              [&full_name](const PythonPartHolder& holder) {
                                  return holder.full_name == full_name;
                              }),
               list.end());
    list.push_back({full_name, factory});
    reg.registerPart(
      part_meta,
      [full_name](const Parameter& scalars, const PartParamMap& parts) -> Part {
          py::gil_scoped_acquire gil;
          py::object factory = findFactory(full_name);
          HKU_CHECK_THROW(!factory.is_none(), std::runtime_error,
                          "The python factory of part \"{}\" has been released", full_name);

          py::dict py_parts;
          for (const auto& [name, part_list] : parts) {
              py::list items;
              for (const auto& p : part_list) {
                  items.append(pywrap::partToPython(p));
              }
              py_parts[py::str(name)] = items;
          }
          py::object ret = factory(scalars, py_parts);
          return pywrap::partFromPython(ret);
      },
      true);
}

void releasePythonParts() {
    if (g_python_parts == nullptr || g_python_parts->empty()) {
        return;
    }
    if (interpreterShuttingDown()) {
        // Leaking is preferred over crashing during the interpreter shutdown
        return;
    }
    py::gil_scoped_acquire gil;
    g_python_parts->clear();
    PartRegistry::instance().unregisterBySource(PartMeta::Source::PYTHON);
}

void export_python_part(py::module& part) {
    // The python side must register through hikyuu.register_python_part (part_bridge.py), which
    // validates the full name and the meta before handing over; this binding is the raw channel
    part.def("_register_python_part", &registerPythonPart, py::arg("full_name"), py::arg("meta"),
             py::arg("factory"), py::arg("overwrite") = false,
             "Register a part whose factory is a python callable factory(scalars, parts) -> part; "
             "an existing registration is rejected unless overwrite is True");
    part.def("release_python_parts", &releasePythonParts,
             "Release the python factories and unregister the python parts; called before the "
             "interpreter shutdown. A part registered with overwrite=True replaces the original "
             "one for good: the original is not restored on release");
}
