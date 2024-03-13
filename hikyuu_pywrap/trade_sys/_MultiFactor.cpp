/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/factor/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyMultiFactor : public MultiFactorBase {
    PY_CLONE(PyMultiFactor, MultiFactorBase)

public:
    using MultiFactorBase::MultiFactorBase;

    IndicatorList _calculate(const vector<IndicatorList>& all_stk_inds) {
        PYBIND11_OVERLOAD_PURE_NAME(IndicatorList, MultiFactorBase, "_calculate", _calculate,
                                    all_stk_inds);
    }
};

void export_MultiFactor(py::module& m) {
    py::class_<MultiFactorBase, MultiFactorPtr, PyMultiFactor>(m, "MultiFactor",
                                                               R"(市场环境判定策略基类

自定义市场环境判定策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())
      //   .def(py::init<const string&>())

      //   .def("__str__", to_py_str<EnvironmentBase>)
      //   .def("__repr__", to_py_str<EnvironmentBase>)

      //   .def_property("name", py::overload_cast<>(&EnvironmentBase::name, py::const_),
      //                 py::overload_cast<const string&>(&EnvironmentBase::name),
      //                 py::return_value_policy::copy, "名称")
      //   .def_property("query", &EnvironmentBase::getQuery, &EnvironmentBase::setQuery,
      //                 py::return_value_policy::copy, "设置或获取查询条件")

      .def("get_param", &MultiFactorBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &MultiFactorBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("haveParam", &MultiFactorBase::haveParam, "是否存在指定参数")

      //   .def("is_valid", &EnvironmentBase::isValid, R"(is_valid(self, datetime)

      // 指定时间系统是否有效

      // :param Datetime datetime: 指定时间
      // :return: True 有效 | False 无效)")

      //   .def("_add_valid", &EnvironmentBase::_addValid, R"(_add_valid(self, datetime)

      // 加入有效时间，在_calculate中调用

      // :param Datetime datetime: 有效时间)")

      //   .def("reset", &EnvironmentBase::reset, "复位操作")
      //   .def("clone", &EnvironmentBase::clone, "克隆操作")
      //   .def("_reset", &EnvironmentBase::_reset,
      //   "【重载接口】子类复位接口，用于复位内部私有变量") .def("_calculate",
      //   &EnvironmentBase::_calculate, "【重载接口】子类计算接口")

      DEF_PICKLE(MultiFactorPtr);
}