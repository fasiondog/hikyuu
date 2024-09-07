/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240907 added by fasiondog
 */

#include <hikyuu/analysis/optimize.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

class PyOptimizeParamInt {
public:
    PyOptimizeParamInt(SystemPart part, const string& name, int start, int end, int step) {
        m_param = std::make_shared<OptimizeParamInt>(part, name, start, end, step);
    }

    OptimizeParamPtr get() {
        return m_param;
    }

    string str() const {
        return m_param->str();
    }

private:
    OptimizeParamPtr m_param;
};

class PyOptimizeParamDouble {
public:
    PyOptimizeParamDouble(SystemPart part, const string& name, double start, double end,
                          double step) {
        m_param = std::make_shared<OptimizeParamDouble>(part, name, start, end, step);
    }

    OptimizeParamPtr get() {
        return m_param;
    }

    string str() const {
        return m_param->str();
    }

private:
    OptimizeParamPtr m_param;
};

class PyOptimizeParamBool {
public:
    PyOptimizeParamBool(SystemPart part, const string& name) {
        m_param = std::make_shared<OptimizeParamBool>(part, name);
    }

    OptimizeParamPtr get() {
        return m_param;
    }

    string str() const {
        return m_param->str();
    }

private:
    OptimizeParamPtr m_param;
};

class PyOptimizeParamString {
public:
    PyOptimizeParamString(SystemPart part, const string& name, const StringList& values) {
        m_param = std::make_shared<OptimizeParamString>(part, name, values);
    }

    OptimizeParamPtr get() {
        return m_param;
    }

    string str() const {
        return m_param->str();
    }

private:
    OptimizeParamPtr m_param;
};

void export_optimize(py::module& m) {
    py::class_<PyOptimizeParamInt>(m, "OptimizeParamInt")
      .def(py::init<SystemPart, const string&, int, int, int>(), py::arg("part"), py::arg("name"),
           py::arg("start"), py::arg("end"), py::arg("step") = 1)
      .def("__str__", &PyOptimizeParamInt::str)
      .def("__call__", &PyOptimizeParamInt::get);

    py::class_<PyOptimizeParamDouble>(m, "OptimizeParamDouble")
      .def(py::init<SystemPart, const string&, double, double, double>(), py::arg("part"),
           py::arg("name"), py::arg("start"), py::arg("end"), py::arg("step"))
      .def("__str__", &PyOptimizeParamDouble::str)
      .def("__call__", &PyOptimizeParamDouble::get);

    py::class_<PyOptimizeParamBool>(m, "OptimizeParamBool")
      .def(py::init<SystemPart, const string&>(), py::arg("part"), py::arg("name"))
      .def("__str__", &PyOptimizeParamBool::str)
      .def("__call__", &PyOptimizeParamBool::get);

    py::class_<PyOptimizeParamString>(m, "OptimizeParamString")
      .def(py::init<SystemPart, const string&, const StringList&>(), py::arg("part"),
           py::arg("name"), py::arg("values"))
      .def("__str__", &PyOptimizeParamString::str)
      .def("__call__", &PyOptimizeParamString::get);

    m.def("find_optimize_param", [](const SYSPtr& sys, const Stock& stk, const KQuery& query,
                                    const py::sequence& opt_params, const string& sort_key) {
        OptimizeParamList c_opt_params;
        auto total = len(opt_params);
        for (auto i = 0; i < total; ++i) {
            const auto& obj = opt_params[i];
            if (py::isinstance<PyOptimizeParamInt>(obj)) {
                c_opt_params.push_back(obj.cast<PyOptimizeParamInt>().get());
            } else if (py::isinstance<PyOptimizeParamDouble>(obj)) {
                c_opt_params.push_back(obj.cast<PyOptimizeParamDouble>().get());
            } else if (py::isinstance<PyOptimizeParamBool>(obj)) {
                c_opt_params.push_back(obj.cast<PyOptimizeParamBool>().get());
            } else if (py::isinstance<PyOptimizeParamDouble>(obj)) {
                c_opt_params.push_back(obj.cast<PyOptimizeParamString>().get());
            } else {
                HKU_THROW("Not OptimizeParam, index: {}", i);
            }
        }
        return findOptimizeParam(sys, stk, query, c_opt_params, sort_key);
    });
}