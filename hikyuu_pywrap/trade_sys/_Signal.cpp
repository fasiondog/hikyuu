/*
 * _Signal.cpp
 *
 *  Created on: 2013-3-18
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/signal/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySignalBase : public SignalBase {
    PY_CLONE(PySignalBase, SignalBase)

public:
    using SignalBase::SignalBase;
    PySignalBase(const SignalBase& base) : SignalBase(base) {}

    void _calculate(const KData& kdata) override {
        PYBIND11_OVERLOAD_PURE(void, SignalBase, _calculate, kdata);
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SignalBase, _reset, );
    }
};

void export_Signal(py::module& m) {
    py::class_<SignalBase, SGPtr, PySignalBase>(m, "SignalBase", py::dynamic_attr(),
                                                R"(信号指示器基类
    信号指示器负责产生买入、卖出信号。

公共参数：

    - alternate (bool|True) ：买入和卖出信号是否交替出现。单线型的信号通常通过拐点、斜率等判断信号的产生，此种情况下可能出现连续出现买入信号或连续出现卖出信号的情况，此时可通过该参数控制买入、卖出信号是否交替出现。而双线交叉型的信号通常本身买入和卖出已经是交替出现，此时该参数无效。

自定义的信号指示器接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")

      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const SignalBase&>())

      .def("__str__", to_py_str<SignalBase>)
      .def("__repr__", to_py_str<SignalBase>)

      .def_property("name", py::overload_cast<>(&SignalBase::name, py::const_),
                    py::overload_cast<const string&>(&SignalBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("to", &SignalBase::getTO, &SignalBase::setTO, py::return_value_policy::copy,
                    "设置或获取交易对象")

      .def("get_param", &SignalBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SignalBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SignalBase::haveParam, "是否存在指定参数")

      .def("should_buy", &SignalBase::shouldBuy, R"(should_buy(self, datetime)

    指定时刻是否可以买入

    :param Datetime datetime: 指定时刻
    :rtype: bool)")

      .def("should_sell", &SignalBase::shouldSell, R"(should_sell(self, datetime)

    指定时刻是否可以卖出

    :param Datetime datetime: 指定时刻
    :rtype: bool)")

      .def("next_time_should_buy", &SignalBase::nextTimeShouldBuy,
           R"(next_time_should_byu(self)

    下一时刻是否可以买入，相当于最后时刻是否指示买入)")

      .def("next_time_should_sell", &SignalBase::nextTimeShouldSell, R"(next_time_should_sell(self)
      
    下一时刻是否可以卖出，相当于最后时刻是否指示卖出)")

      .def("get_buy_signal", &SignalBase::getBuySignal, R"(get_buy_signal(self)

    获取所有买入指示日期列表
    
    :rtype: DatetimeList)")

      .def("get_sell_signal", &SignalBase::getSellSignal, R"(get_sell_signal(self)

    获取所有卖出指示日期列表

    :rtype: DatetimeList)")

      .def("_add_signal", &SignalBase::_addSignal, py::arg("datetime"), py::arg("value"), R"()")

      .def("_add_buy_signal", &SignalBase::_addBuySignal, py::arg("datetime"),
           py::arg("value") = 1.0,
           R"(_add_buy_signal(self, datetime)

    加入买入信号，在_calculate中调用

    :param Datetime datetime: 指示买入的日期)")

      .def("_add_sell_signal", &SignalBase::_addSellSignal, py::arg("datetime"),
           py::arg("value") = -1.0, R"(_add_sell_signal(self, datetime)

    加入卖出信号，在_calculate中调用

    :param Datetime datetime: 指示卖出的日期)")

      .def("reset", &SignalBase::reset, "复位操作")
      .def("clone", &SignalBase::clone, "克隆操作")
      .def("_calculate", &SignalBase::_calculate, R"(_calculate(self, kdata)
      
    【重载接口】子类计算接口)")

      .def("_reset", &SignalBase::_reset, "【重载接口】子类复位接口，复位内部私有变量")

      .def("__add__", [](const SignalPtr& self, const SignalPtr& other) { return self + other; })
      .def("__add__", [](const SignalPtr& self, double other) { return self + other; })
      .def("__radd__", [](const SignalPtr& self, double other) { return other + self; })
      .def("__sub__", [](const SignalPtr& self, const SignalPtr& other) { return self - other; })
      .def("__sub__", [](const SignalPtr& self, double other) { return self - other; })
      .def("__rsub__", [](const SignalPtr& self, double other) { return other - self; })
      .def("__mul__", [](const SignalPtr& self, const SignalPtr& other) { return self * other; })
      .def("__mul__", [](const SignalPtr& self, double other) { return self * other; })
      .def("__rmul__", [](const SignalPtr& self, double other) { return other * self; })
      .def("__truediv__",
           [](const SignalPtr& self, const SignalPtr& other) { return self / other; })
      .def("__truediv__", [](const SignalPtr& self, double other) { return self / other; })
      .def("__rtruediv__", [](const SignalPtr& self, double other) { return other / self; })
      .def("__and__", [](const SignalPtr& self, const SignalPtr& other) { return self & other; })
      .def("__or__", [](const SignalPtr& self, const SignalPtr& other) { return self | other; })

        DEF_PICKLE(SGPtr);

    m.def("SG_Bool", SG_Bool, py::arg("buy"), py::arg("sell"), py::arg("alternate") = true,
          R"(SG_Bool(buy, sell)

    布尔信号指示器，使用运算结果为类似bool数组的Indicator分别作为买入、卖出指示。

    :param Indicator buy: 买入指示（结果Indicator中相应位置>0则代表买入）
    :param Indicator sell: 卖出指示（结果Indicator中相应位置>0则代表卖出）
    :param bool alternate: 是否交替买入卖出，默认为True
    :return: 信号指示器)");

    m.def("SG_Single", SG_Single, py::arg("ind"), py::arg("filter_n") = 10,
          py::arg("filter_p") = 0.1,
          R"(SG_Single(ind[, filter_n = 10, filter_p = 0.1])
    
    生成单线拐点信号指示器。使用《精明交易者》 [BOOK1]_ 中给出的曲线拐点算法判断曲线趋势，公式见下::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Indicator ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :return: 信号指示器)");

    m.def("SG_Single2", SG_Single2, py::arg("ind"), py::arg("filter_n") = 10,
          py::arg("filter_p") = 0.1,
          R"(SG_Single2(ind[, filter_n = 10, filter_p = 0.1])
    
    生成单线拐点信号指示器2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Indicator ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :return: 信号指示器)");

    m.def("SG_Cross", SG_Cross, py::arg("fast"), py::arg("slow"),
          R"(SG_Cross(fast, slow)

    双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: 快线
    :param Indicator slow: 慢线
    :return: 信号指示器)");

    m.def("SG_CrossGold", SG_CrossGold, py::arg("fast"), py::arg("slow"),
          R"(SG_CrossGold(fast, slow)

    金叉指示器，当快线从下向上穿越慢线且快线和慢线的方向都是向上时为金叉，买入；
    当快线从上向下穿越慢线且快线和慢线的方向都是向下时死叉，卖出。::

        SG_CrossGold(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: 快线
    :param Indicator slow: 慢线
    :return: 信号指示器)");

    m.def("SG_Flex", SG_Flex, py::arg("op"), py::arg("slow_n"),
          R"(SG_Flex(ind, slow_n)

    使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。

    :param Indicator ind:
    :param int slow_n: 慢线EMA周期
    :return: 信号指示器)");

    m.def("SG_Band",
          py::overload_cast<const Indicator&, const Indicator&, const Indicator&>(SG_Band),
          py::arg("ind"), py::arg("lower"), py::arg("upper"));
    m.def("SG_Band", py::overload_cast<const Indicator&, price_t, price_t>(SG_Band), py::arg("ind"),
          py::arg("lower"), py::arg("upper"),
          R"(SG_Band(ind, lower, upper)
          
    指标区间指示器, 当指标超过上轨时，买入；
    当指标低于下轨时，卖出。::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH)))");

    m.def("SG_AllwaysBuy", SG_AllwaysBuy, R"(SG_AllwaysBuy()
    
    一个特殊的SG，持续每天发出买入信号，通常配合 PF 使用)");

    m.def("SG_Cycle", SG_Cycle, R"(SG_Cycle()
    
    一个特殊的SG，配合PF使用，以 PF 调仓周期为买入信号)");

    m.def("SG_OneSide", SG_OneSide, py::arg("ind"), py::arg("is_buy"),
          R"(SG_OneSide(ind, is_buy)
          
    根据输入指标构建单边信号（单纯的只包含买入或卖出信号），如果指标值大于0，则加入信号
    
    :param Indicator ind: 输入指标
    :param bool is_buy: 构建的是买入信号，否则为卖出信号
    :return: 信号指示器)");
}
