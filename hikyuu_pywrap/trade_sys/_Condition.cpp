/*
 * _Condition.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/condition/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyConditionBase : public ConditionBase {
    PY_CLONE(PyConditionBase, ConditionBase)

public:
    using ConditionBase::ConditionBase;

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, ConditionBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, ConditionBase, _reset, );
    }
};

void export_Condition(py::module& m) {
    py::class_<ConditionBase, ConditionPtr, PyConditionBase>(
      m, "ConditionBase",
      R"(系统有效条件基类自定义系统有效条件接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<ConditionBase>)
      .def("__repr__", to_py_str<ConditionBase>)

      .def_property("name", py::overload_cast<>(&ConditionBase::name, py::const_),
                    py::overload_cast<const string&>(&ConditionBase::name),
                    py::return_value_policy::copy, "名称")

      .def_property("to", &ConditionBase::getTO, &ConditionBase::setTO, "设置或获取交易对象")
      .def_property("tm", &ConditionBase::getTM, &ConditionBase::setTM, "设置或获取交易管理账户")
      .def_property("sg", &ConditionBase::getSG, &ConditionBase::setSG, "设置或获取交易信号指示器")

      .def("get_param", &ConditionBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &ConditionBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &ConditionBase::haveParam, "是否存在指定参数")

      .def("is_valid", &ConditionBase::isValid, R"(is_valid(self, datetime)

    指定时间系统是否有效

    :param Datetime datetime: 指定时间
    :return: True 有效 | False 无效)")

      .def("reset", &ConditionBase::reset, "复位操作")
      .def("clone", &ConditionBase::clone, "克隆操作")

      .def("get_datetime_list", &ConditionBase::getDatetimeList, R"(get_datetime_list(self)
    
    获取系统有效的日期。注意仅返回系统有效的日期列表，和交易对象不等长)")

      .def("get_values", &ConditionBase::getValues, R"(get_values(self)
           
    以指标的形式获取实际值，与交易对象等长，0表示无效，1表示系统有效)")

      .def("_add_valid", &ConditionBase::_addValid, R"(_add_valid(self, datetime)

    加入有效时间，在_calculate中调用

    :param Datetime datetime: 有效时间)")

      .def("_calculate", &ConditionBase::_calculate, "【重载接口】子类计算接口")
      .def("_reset", &ConditionBase::_reset, "【重载接口】子类复位接口，复位内部私有变量")

        DEF_PICKLE(ConditionPtr);

    m.def("CN_OPLine", CN_OPLine, R"(CN_OPLine(ind)

    固定使用股票最小交易量进行交易，计算权益曲线的ind值，当权益曲线高于ind时，系统有效，否则无效。

    :param Indicator ind: Indicator实例
    :return: 系统有效条件实例
    :rtype: ConditionBase)");

    m.def("CN_Bool", CN_Bool, R"(CN_Bool(ind)

    布尔信号指标系统有效条件, 指标中相应位置>0则代表系统有效，否则无效

    :param Indicator ind: bool型指标，输入为 KData
    :return: 系统有效条件实例
    :rtype: ConditionBase)");
}
