/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/profitgoal/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class ProfitGoalWrap : public ProfitGoalBase, public wrapper<ProfitGoalBase> {
public:
    ProfitGoalWrap() : ProfitGoalBase() {}
    ProfitGoalWrap(const string& name) : ProfitGoalBase(name) {}
    virtual ~ProfitGoalWrap() {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            ProfitGoalBase::_reset();
        }
    }

    void default_reset() {
        this->ProfitGoalBase::_reset();
    }

    ProfitGoalPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    void buyNotify(const TradeRecord& tr) {
        if (override buy_notify = this->get_override("buyNotify")) {
            buy_notify(tr);
            return;
        }

        this->ProfitGoalBase::buyNotify(tr);
    }

    void default_buyNotify(const TradeRecord& tr) {
        this->ProfitGoalBase::buyNotify(tr);
    }

    void sellNotify(const TradeRecord& tr) {
        if (override sell_notify = this->get_override("sellNotify")) {
            sell_notify(tr);
            return;
        }

        this->ProfitGoalBase::sellNotify(tr);
    }

    void default_sellNotify(const TradeRecord& tr) {
        this->ProfitGoalBase::sellNotify(tr);
    }

    price_t getGoal(const Datetime& datetime, price_t price) {
        return this->get_override("getGoal")(datetime, price);
    }

    price_t getShortGoal(const Datetime& datetime, price_t price) {
        if (override getShortGoal = get_override("getShortGoal")) {
            return getShortGoal(datetime, price);
        }
        return ProfitGoalBase::getShortGoal(datetime, price);
    }

    price_t default_getShortGoal(const Datetime& datetime, price_t price) {
        return this->ProfitGoalBase::getShortGoal(datetime, price);
    }
};

string (ProfitGoalBase::*pg_get_name)() const = &ProfitGoalBase::name;
void (ProfitGoalBase::*pg_set_name)(const string&) = &ProfitGoalBase::name;

void export_ProfitGoal() {
    class_<ProfitGoalWrap, boost::noncopyable>("ProfitGoalBase", R"(盈利目标策略基类
    
自定义盈利目标策略接口：

- getGoal : 【必须】获取目标价格
- _calculate : 【必须】子类计算接口
- _clone : 【必须】克隆接口
- _reset : 【可选】重载私有变量
- buyNotify : 【可选】接收实际买入通知，预留用于多次增减仓处理
- sellNotify : 【可选】接收实际卖出通知，预留用于多次增减仓处理)",
                                               init<>())

      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", pg_get_name, pg_set_name, "名称")
      .add_property("to", &ProfitGoalBase::getTO, &ProfitGoalBase::setTO, "设置或获取交易对象")
      .add_property("tm", &ProfitGoalBase::getTM, &ProfitGoalBase::setTM, "设置或获取交易管理账户")

      .def("get_param", &ProfitGoalBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &ProfitGoalBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &ProfitGoalBase::haveParam, "是否存在指定参数")

      .def("buy_notify", &ProfitGoalBase::buyNotify, &ProfitGoalWrap::default_buyNotify,
           R"(buy_notify(self, trade_record)
    
    【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载

    :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录)")

      .def("sell_notify", &ProfitGoalBase::sellNotify, &ProfitGoalWrap::default_sellNotify,
           R"(sell_notify(self, trade_record)
    
    【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
    :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录)")

      .def("get_goal", pure_virtual(&ProfitGoalBase::getGoal), R"(get_goal(self, datetime, price)

    【重载接口】获取盈利目标价格，返回constant.null_price时，表示未限定目标；返回0意味着需要卖出

    :param Datetime datetime: 买入时间
    :param float price: 买入价格
    :return: 目标价格
    :rtype: float)")

      //.def("getShortGoal", &ProfitGoalBase::getShortGoal, &ProfitGoalWrap::default_getShortGoal)

      .def("reset", &ProfitGoalBase::reset, "复位操作")
      .def("clone", &ProfitGoalBase::clone, "克隆操作")
      .def("_calculate", pure_virtual(&ProfitGoalBase::_calculate), "【重载接口】子类计算接口")
      .def("_reset", &ProfitGoalBase::_reset, &ProfitGoalWrap::default_reset,
           "【重载接口】子类复位接口，复位内部私有变量")
      .def("_clone", pure_virtual(&ProfitGoalBase::_clone), "【重载接口】子类克隆接口");

    register_ptr_to_python<ProfitGoalPtr>();

    def("PG_NoGoal", PG_NoGoal, R"(PG_NoGoal()

    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例)");

    def("PG_FixedPercent", PG_FixedPercent, (arg("p") = 0.2), R"(PG_FixedPercent([p = 0.2])

    固定百分比盈利目标，目标价格 = 买入价格 * (1 + p)
    
    :param float p: 百分比
    :return: 盈利目标策略实例)");

    def("PG_FixedHoldDays", PG_FixedHoldDays, (arg("days") = 5), R"(PG_FixedHoldDays([days=5])

    固定持仓天数盈利目标策略
    
    :param int days: 允许持仓天数（按交易日算）,默认5天
    :return: 盈利目标策略实例)");
}
