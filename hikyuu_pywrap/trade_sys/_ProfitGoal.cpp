/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/profitgoal/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyProfitGoalBase : public ProfitGoalBase {
    PY_CLONE(PyProfitGoalBase, ProfitGoalBase)

public:
    using ProfitGoalBase::ProfitGoalBase;
    PyProfitGoalBase(const ProfitGoalBase& base) : ProfitGoalBase(base) {}

    void buyNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "buy_notify", buyNotify, tr);
    }

    void sellNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "sell_notify", sellNotify, tr);
    }

    price_t getGoal(const Datetime& datetime, price_t price) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, ProfitGoalBase, "get_goal", getGoal, datetime, price);
    }

    price_t getShortGoal(const Datetime& date, price_t price) override {
        PYBIND11_OVERLOAD_NAME(price_t, ProfitGoalBase, "get_short_goal", getShortGoal, date,
                               price);
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, ProfitGoalBase, _reset, );
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, ProfitGoalBase, _calculate, );
    };
};

void export_ProfitGoal(py::module& m) {
    py::class_<ProfitGoalBase, PGPtr, PyProfitGoalBase>(m, "ProfitGoalBase",
                                                        R"(盈利目标策略基类
    
自定义盈利目标策略接口：

- getGoal : 【必须】获取目标价格
- _calculate : 【必须】子类计算接口
- _clone : 【必须】克隆接口
- _reset : 【可选】重载私有变量
- buyNotify : 【可选】接收实际买入通知，预留用于多次增减仓处理
- sellNotify : 【可选】接收实际卖出通知，预留用于多次增减仓处理)")

      .def(py::init<>())
      .def(py::init<const ProfitGoalBase&>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<ProfitGoalBase>)
      .def("__repr__", to_py_str<ProfitGoalBase>)

      .def_property("name", py::overload_cast<>(&ProfitGoalBase::name, py::const_),
                    py::overload_cast<const string&>(&ProfitGoalBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("to", &ProfitGoalBase::getTO, &ProfitGoalBase::setTO, "设置或获取交易对象")
      .def_property("tm", &ProfitGoalBase::getTM, &ProfitGoalBase::setTM, "设置或获取交易管理账户")

      .def("get_param", &ProfitGoalBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &ProfitGoalBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &ProfitGoalBase::haveParam, "是否存在指定参数")

      .def("buy_notify", &ProfitGoalBase::buyNotify,
           R"(buy_notify(self, trade_record)
    
    【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载

    :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录)")

      .def("sell_notify", &ProfitGoalBase::sellNotify,
           R"(sell_notify(self, trade_record)
    
    【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
    :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录)")

      .def("get_goal", &ProfitGoalBase::getGoal, R"(get_goal(self, datetime, price)

    【重载接口】获取盈利目标价格，返回constant.null_price时，表示未限定目标；返回0意味着需要卖出

    :param Datetime datetime: 买入时间
    :param float price: 买入价格
    :return: 目标价格
    :rtype: float)")

      //.def("getShortGoal", &ProfitGoalBase::getShortGoal, &ProfitGoalWrap::default_getShortGoal)

      .def("reset", &ProfitGoalBase::reset, "复位操作")
      .def("clone", &ProfitGoalBase::clone, "克隆操作")
      .def("_calculate", &ProfitGoalBase::_calculate, "【重载接口】子类计算接口")
      .def("_reset", &ProfitGoalBase::_reset, "【重载接口】子类复位接口，复位内部私有变量")

        DEF_PICKLE(PGPtr);

    m.def("PG_NoGoal", PG_NoGoal, R"(PG_NoGoal()

    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例)");

    m.def("PG_FixedPercent", PG_FixedPercent, py::arg("p") = 0.2, R"(PG_FixedPercent([p = 0.2])

    固定百分比盈利目标，目标价格 = 买入价格 * (1 + p)
    
    :param float p: 百分比
    :return: 盈利目标策略实例)");

    m.def("PG_FixedHoldDays", PG_FixedHoldDays, py::arg("days") = 5, R"(PG_FixedHoldDays([days=5])

    固定持仓天数盈利目标策略
    
    :param int days: 允许持仓天数（按交易日算）,默认5天
    :return: 盈利目标策略实例)");
}
