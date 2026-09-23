/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-04
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/multifactor/buildin_norm.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyNormalizeBase : public NormalizeBase {
    PY_CLONE(PyNormalizeBase, NormalizeBase)

public:
    PyNormalizeBase() : NormalizeBase() {
        m_is_python_object = true;
    }

    PyNormalizeBase(const string& name) : NormalizeBase(name) {
        m_is_python_object = true;
    }

    PyNormalizeBase(const NormalizeBase& base) : NormalizeBase(base) {
        m_is_python_object = true;
    }

    PriceList normalize(const PriceList& data) override {
        PYBIND11_OVERRIDE_PURE(PriceList, NormalizeBase, normalize, data);
    }
};

void export_Normlize(py::module& m) {
    py::class_<NormalizeBase, NormalizePtr, PyNormalizeBase>(m, "NormalizeBase", py::dynamic_attr(),
                                                             R"(The cross-section standardization operation used for the MF)")
      .def(py::init<>())
      .def(py::init<const NormalizeBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<NormalizeBase>)
      .def("__repr__", to_py_str<NormalizeBase>)

      .def_property("name", py::overload_cast<>(&NormalizeBase::name, py::const_),
                    py::overload_cast<const string&>(&NormalizeBase::name),
                    py::return_value_policy::copy, "Name")

      .def("get_param", &NormalizeBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (NormalizeBase::*)(const std::string&, const boost::any&)>(
             &NormalizeBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &NormalizeBase::haveParam, "Whether the specified parameter exists")

      .def("clone", &NormalizeBase::clone, "The clone operation")

      .def("normalize", &NormalizeBase::normalize, "[Overload interface] The subclass calculation interface")

        DEF_PICKLE(NormalizePtr);

    m.def("NORM_NOTHING", &NORM_NOTHING, "No cross-section standardization operation");
    m.def("NORM_MinMax", &NORM_MinMax, "The min-max standardization operation");
    m.def("NORM_Zscore", &NORM_Zscore, py::arg("out_extreme") = false, py::arg("nsigma") = 3.0,
          py::arg("recursive") = false, R"(The Z-score standardization operation
          
    :param out_extreme: whether to remove the outliers
    :param nsigma: the outlier judgment multiple ±3.0
    :param recursive: whether to process the outliers recursively)");

    m.def("NORM_Quantile", &NORM_Quantile, py::arg("quantile_min") = 0.01,
          py::arg("quantile_max") = 0.99, R"(The quantile cross-section standardization operation
          
    :param quantile_min: the minimum quantile
    :param quantile_max: the maximum quantile)");

    m.def("NORM_Quantile_Uniform", &NORM_Quantile_Uniform, py::arg("quantile_min") = 0.01,
          py::arg("quantile_max") = 0.99, R"(The quantile cross-section uniform distribution standardization operation
          
    :param quantile_min: the minimum quantile
    :param quantile_max: the maximum quantile)");
}