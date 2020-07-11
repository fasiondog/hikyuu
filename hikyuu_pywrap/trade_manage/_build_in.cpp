/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/build_in.h>

using namespace boost::python;
using namespace hku;

void export_build_in() {
    def(
      "crtTM", crtTM,
      (arg("date") = Datetime(199001010000LL), arg("init_cash") = 100000,
       arg("cost_func") = TC_Zero(), arg("name") = "SYS"),
      R"(crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS"])

    创建交易管理模块，管理帐户的交易记录及资金使用情况
    
    :param Datetime date:  账户建立日期
    :param float init_cash:    初始资金
    :param TradeCost cost_func: 交易成本算法
    :param string name:        账户名称
    :rtype: TradeManager)");

    def("TC_TestStub", TC_TestStub, "仅用于测试");

    def(
      "TC_FixedA", TC_FixedA,
      (arg("commission") = 0.0018, arg("lowest_commission") = 5.0, arg("stamptax") = 0.001,
       arg("transferfee") = 0.001, arg("lowest_transferfee") = 1.0),
      R"(TC_FixedA([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.001, lowestTransferfee=1.0])

    2015年8月1日之前的A股交易成本算法

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :param float lowestTransferfee: 最低过户费
    :return: :py:class:`TradeCostBase` 子类实例)");

    def(
      "TC_FixedA2015", TC_FixedA2015,
      (arg("commission") = 0.0018, arg("lowest_commission") = 5.0, arg("stamptax") = 0.001,
       arg("transferfee") = 0.00002),
      R"(TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    2015年8月1日上证过户费改为成交金额的千分之0.02

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例)");

    def(
      "TC_FixedA2017", TC_FixedA2017,
      (arg("commission") = 0.0018, arg("lowest_commission") = 5.0, arg("stamptax") = 0.001,
       arg("transferfee") = 0.00002),
      R"(TC_FixedA2015([commission=0.0018, lowestCommission=5.0, stamptax=0.001, transferfee=0.00002])

    2017年1月1日起将对深市过户费项目单独列示，标准为成交金额0.02‰双向收取。

    :param float commission: 佣金比例
    :param float lowestCommission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例)");

    def("TC_Zero", TC_Zero, "零交易成本算法");
}
