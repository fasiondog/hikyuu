/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SlippageWrap : public SlippageBase, public wrapper<SlippageBase> {
public:
    SlippageWrap() : SlippageBase() {}
    SlippageWrap(const string& name) : SlippageBase(name) {}
    virtual ~SlippageWrap() {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            SlippageBase::_reset();
        }
    }

    void default_reset() {
        this->SlippageBase::_reset();
    }

    SlippagePtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate");
    }

    price_t getRealBuyPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getRealBuyPrice")(datetime, price);
    }

    price_t getRealSellPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getRealSellPrice")(datetime, price);
    }
};

string (SlippageBase::*sp_get_name)() const = &SlippageBase::name;
void (SlippageBase::*sp_set_name)(const string&) = &SlippageBase::name;

void export_Slippage() {
    class_<SlippageWrap, boost::noncopyable>("SlippageBase", R"(移滑价差算法基类

自定义移滑价差接口：

    - getRealBuyPrice : 【必须】计算实际买入价格
    - getRealSellPrice : 【必须】计算实际卖出价格
    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)",
                                             init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", sp_get_name, sp_set_name, "名称")
      .add_property("to", &SlippageBase::getTO, &SlippageBase::setTO, "设置或获取交易对象")

      .def("get_param", &SlippageBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SlippageBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SlippageBase::haveParam, "是否存在指定参数")

      .def("get_real_buy_price", pure_virtual(&SlippageBase::getRealBuyPrice),
           R"(get_real_buy_price(self, datetime, price)

    【重载接口】计算实际买入价格

    :param Datetime datetime: 买入时间
    :param float price: 计划买入价格
    :return: 实际买入价格
    :rtype: float)")

      .def("get_real_sell_price", pure_virtual(&SlippageBase::getRealSellPrice),
           R"(get_real_sell_price(self, datetime, price)

    【重载接口】计算实际卖出价格

    :param Datetime datetime: 卖出时间
    :param float price: 计划卖出价格
    :return: 实际卖出价格
    :rtype: float)")

      .def("reset", &SlippageBase::reset, "复位操作")
      .def("clone", &SlippageBase::clone, "克隆操作")
      .def("_calculate", pure_virtual(&SlippageBase::_calculate), "【重载接口】子类计算接口")
      .def("_reset", &SlippageBase::_reset, &SlippageWrap::default_reset,
           "【重载接口】子类复位接口，复位内部私有变量")
      .def("_clone", pure_virtual(&SlippageBase::_clone), "【重载接口】子类克隆接口");

    register_ptr_to_python<SlippagePtr>();

    def("SP_FixedPercent", SP_FixedPercent, (arg("p") = 0.001), R"(SP_FixedPercent([p=0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)

    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例)");

    def("SP_FixedValue", SP_FixedValue, (arg("value") = 0.01), R"(SP_FixedValuet([p=0.001])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格

    :param float p: 偏移价格
    :return: 移滑价差算法实例)");
}
