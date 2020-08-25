/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/stoploss/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class StoplossWrap : public StoplossBase, public wrapper<StoplossBase> {
public:
    StoplossWrap() : StoplossBase() {}
    StoplossWrap(const string& name) : StoplossBase(name) {}
    virtual ~StoplossWrap() {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            StoplossBase::_reset();
        }
    }

    void default_reset() {
        this->StoplossBase::_reset();
    }

    StoplossPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    price_t getPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getPrice")(datetime, price);
    }

    price_t getShortPrice(const Datetime& datetime, price_t price) {
        if (override getShortPrice = get_override("getShortPrice")) {
            return getShortPrice(datetime, price);
        }
        return StoplossBase::getShortPrice(datetime, price);
    }

    price_t default_getShortPrice(const Datetime& datetime, price_t price) {
        return this->StoplossBase::getShortPrice(datetime, price);
    }
};

string (StoplossBase::*st_get_name)() const = &StoplossBase::name;
void (StoplossBase::*st_set_name)(const string&) = &StoplossBase::name;

void export_Stoploss() {
    class_<StoplossWrap, boost::noncopyable>("StoplossBase", R"(止损/止赢算法基类
自定义止损/止赢策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)",
                                             init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", st_get_name, st_set_name, "名称")
      .add_property("to", &StoplossBase::getTO, &StoplossBase::setTO, "设置或获取交易对象")
      .add_property("tm", &StoplossBase::getTM, &StoplossBase::setTM, "设置或获取交易管理实例")

      .def("get_param", &StoplossBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &StoplossBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &StoplossBase::haveParam, "是否存在指定参数")

      .def("get_price", pure_virtual(&StoplossBase::getPrice), R"(get_price(self, datetime, price)

    【重载接口】获取本次预期交易（买入）时的计划止损价格，如果不存在止损价，则返回0。用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。

    .. note::
        一般情况下，止损/止赢的算法可以互换，但止损的getPrice可以传入计划交易的价格，比如以买入价格的30%做为止损。而止赢则不考虑传入的price参数，即认为price为0.0。实际上，即使止损也不建议使用price参数，如可以使用前日最低价的30%作为止损，则不需要考虑price参数。

    :param Datetime datetime: 交易时间
    :param float price: 计划买入的价格
    :return: 止损价格
    :rtype: float)")

      //.def("getShortPrice", &StoplossBase::getShortPrice, &StoplossWrap::default_getShortPrice)

      .def("reset", &StoplossBase::reset, "复位操作")
      .def("clone", &StoplossBase::clone, "克隆操作")
      .def("_calculate", pure_virtual(&StoplossBase::_calculate), "【重载接口】子类计算接口")
      .def("_reset", &StoplossBase::_reset, &StoplossWrap::default_reset,
           "【重载接口】子类复位接口，复位内部私有变量")
      .def("_clone", pure_virtual(&StoplossBase::_clone), "【重载接口】子类克隆接口")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<StoplossBase>())
#endif
      ;

    register_ptr_to_python<StoplossPtr>();

    def("ST_FixedPercent", ST_FixedPercent, (arg("p") = 0.03), R"(ST_FixedPercent([p=0.03])

    固定百分比止损策略，即当价格低于买入价格的某一百分比时止损

    :param float p: 百分比(0,1]
    :return: 止损/止赢策略实例)");

    def("ST_Indicator", ST_Indicator, (arg("op"), arg("kpart") = "CLOSE"),
        R"(ST_Indicator(op[, kpart="CLOSE"])

    使用技术指标作为止损价。如使用10日EMA作为止损：::

        ST_Indicator(OP(EMA(n=10)))

    :param Indicator op:
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 止损/止赢策略实例)");

    def("ST_Saftyloss", ST_Saftyloss, (arg("n1") = 10, arg("n2") = 3, arg("p") = 2.0),
        R"(ST_Saftyloss([n1=10, n2=3, p=2.0])

    参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
    得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
    最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
    上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param int n1: 计算平均噪音的回溯时间窗口，默认为10天
    :param int n2: 对初步止损线去n2日内的最高值，默认为3
    :param double p: 噪音系数，默认为2
    :return: 止损/止赢策略实例)");
}
