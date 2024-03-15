/*
 * _Environment.cpp
 *
 *  Created on: 2013-3-2
 *      Author: fasiondog
 */

/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/environment/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyEnvironmentBase : public EnvironmentBase {
    PY_CLONE(PyEnvironmentBase, EnvironmentBase)

public:
    using EnvironmentBase::EnvironmentBase;

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, EnvironmentBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, EnvironmentBase, _reset, );
    }
};

void export_Environment(py::module& m) {
    py::class_<EnvironmentBase, EnvironmentPtr, PyEnvironmentBase>(
      m, "EnvironmentBase",
      R"(市场环境判定策略基类

自定义市场环境判定策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())
      .def(py::init<const string&>())

      .def("__str__", to_py_str<EnvironmentBase>)
      .def("__repr__", to_py_str<EnvironmentBase>)

      .def_property("name", py::overload_cast<>(&EnvironmentBase::name, py::const_),
                    py::overload_cast<const string&>(&EnvironmentBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("query", &EnvironmentBase::getQuery, &EnvironmentBase::setQuery,
                    py::return_value_policy::copy, "设置或获取查询条件")

      .def("get_param", &EnvironmentBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &EnvironmentBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &EnvironmentBase::haveParam, "是否存在指定参数")

      .def("is_valid", &EnvironmentBase::isValid, R"(is_valid(self, datetime)

    指定时间系统是否有效

    :param Datetime datetime: 指定时间
    :return: True 有效 | False 无效)")

      .def("_add_valid", &EnvironmentBase::_addValid, R"(_add_valid(self, datetime)

    加入有效时间，在_calculate中调用

    :param Datetime datetime: 有效时间)")

      .def("reset", &EnvironmentBase::reset, "复位操作")
      .def("clone", &EnvironmentBase::clone, "克隆操作")
      .def("_reset", &EnvironmentBase::_reset, "【重载接口】子类复位接口，用于复位内部私有变量")
      .def("_calculate", &EnvironmentBase::_calculate, "【重载接口】子类计算接口")

        DEF_PICKLE(EnvironmentPtr);

    m.def("EV_TwoLine", EV_TwoLine, py::arg("fast"), py::arg("slow"), py::arg("market") = "SH",
          R"(EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Indicator fast: 快线指标
    :param Indicator slow: 慢线指标
    :param string market: 市场名称)");

    m.def("EV_Bool", EV_Bool, py::arg("ind"), py::arg("market") = "SH",
          R"(EV_Bool(ind, market='SH')

    布尔信号指标市场环境

    :param Indicator ind: bool类型的指标，指标中相应位置>0则代表市场有效，否则无效
    :param str market: 指定的市场，用于获取相应的交易日历)");
}
