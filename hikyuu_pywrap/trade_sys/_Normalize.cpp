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
                                                             R"(用于 MF 的截面标准化操作)")
      .def(py::init<>())
      .def(py::init<const NormalizeBase&>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<NormalizeBase>)
      .def("__repr__", to_py_str<NormalizeBase>)

      .def_property("name", py::overload_cast<>(&NormalizeBase::name, py::const_),
                    py::overload_cast<const string&>(&NormalizeBase::name),
                    py::return_value_policy::copy, "名称")

      .def("get_param", &NormalizeBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &NormalizeBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &NormalizeBase::haveParam, "是否存在指定参数")

      .def("clone", &NormalizeBase::clone, "克隆操作")

      .def("normalize", &NormalizeBase::normalize, "【重载接口】子类计算接口")

        DEF_PICKLE(NormalizePtr);

    m.def("NORM_NOTHING", &NORM_NOTHING, "无截面标准化操作");
    m.def("NORM_MinMax", &NORM_MinMax, "最小-最大标准化操作");
    m.def("NORM_Zscore", &NORM_Zscore, py::arg("out_extreme") = false, py::arg("nsigma") = 3.0,
          py::arg("recursive") = false, R"(Z-score 标准化操作
          
    :param out_extreme: 是否剔除异常值
    :param nsigma: 异常值判断倍数±3.0
    :param recursive: 是否递归处理异常值)");

    m.def("NORM_Quantile", &NORM_Quantile, py::arg("quantile_min") = 0.01,
          py::arg("quantile_max") = 0.99, R"(分位数截面标准化操作
          
    :param quantile_min: 最小分位数
    :param quantile_max: 最大分位数)");

    m.def("NORM_Quantile_Uniform", &NORM_Quantile_Uniform, py::arg("quantile_min") = 0.01,
          py::arg("quantile_max") = 0.99, R"(分位数截面均匀分布标准化操作
          
    :param quantile_min: 最小分位数
    :param quantile_max: 最大分位数)");
}