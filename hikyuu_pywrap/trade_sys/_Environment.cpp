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

#include <boost/python.hpp>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/environment/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class EnvironmentWrap : public EnvironmentBase, public wrapper<EnvironmentBase> {
public:
    EnvironmentWrap() : EnvironmentBase() {}
    EnvironmentWrap(const string& name) : EnvironmentBase(name) {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            EnvironmentBase::_reset();
        }
    }

    void default_reset() {
        this->EnvironmentBase::_reset();
    }

    EnvironmentPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }
};

string (EnvironmentBase::*ev_get_name)() const = &EnvironmentBase::name;
void (EnvironmentBase::*ev_set_name)(const string&) = &EnvironmentBase::name;

void export_Environment() {
    class_<EnvironmentWrap, boost::noncopyable>("EnvironmentBase",
                                                R"(市场环境判定策略基类

自定义市场环境判定策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)",
                                                init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", ev_get_name, ev_set_name, "名称")
      .add_property("query", &EnvironmentBase::getQuery, &EnvironmentBase::setQuery,
                    "设置或获取查询条件")

      .def("get_param", &EnvironmentBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &EnvironmentBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("haveParam", &EnvironmentBase::haveParam, "是否存在指定参数")

      .def("is_valid", &EnvironmentBase::isValid, R"(is_valid(self, datetime)

    指定时间系统是否有效

    :param Datetime datetime: 指定时间
    :return: True 有效 | False 无效)")

      .def("_add_valid", &EnvironmentBase::_addValid, R"(_add_valid(self, datetime)

    加入有效时间，在_calculate中调用

    :param Datetime datetime: 有效时间)")

      .def("reset", &EnvironmentBase::reset, "复位操作")
      .def("clone", &EnvironmentBase::clone, "克隆操作")
      .def("_reset", &EnvironmentBase::_reset, &EnvironmentWrap::default_reset,
           "【重载接口】子类复位接口，用于复位内部私有变量")
      .def("_clone", pure_virtual(&EnvironmentBase::_clone), "【重载接口】子类克隆接口")
      .def("_calculate", pure_virtual(&EnvironmentBase::_calculate), "【重载接口】子类计算接口");

    register_ptr_to_python<EnvironmentPtr>();

    def("EV_TwoLine", EV_TwoLine, (arg("fast"), arg("slow"), arg("market") = "SH"),
        R"(EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Indicator fast: 快线指标
    :param Indicator slow: 慢线指标
    :param string market: 市场名称)");
}
