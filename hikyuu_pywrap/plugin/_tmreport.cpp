/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-27
 *      Author: fasiondog
 */

#include <csignal>
#include <hikyuu/plugin/tmreport.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_tmreport(py::module& m) {
    py::class_<PositionExtInfo>(m, "PositionExtInfo", "持仓记录扩展信息")
      .def(py::init<>())

      //   .def("__str__", &PositionRecord::str)
      //   .def("__repr__", &PositionRecord::str)

      .def_readonly("position", &PositionExtInfo::position, "持仓记录")
      .def_readonly("max_high_price", &PositionExtInfo::maxHighPrice, "期间最高价最大值")
      .def_readonly("min_low_price", &PositionExtInfo::minLowPrice, "期间最低价最小值")
      .def_readonly("max_close_price", &PositionExtInfo::maxClosePrice, "期间收盘价最高值")
      .def_readonly("min_close_price", &PositionExtInfo::minClosePrice, "期间收盘价最低值")
      .def_readonly("current_close_price", &PositionExtInfo::currentClosePrice, "当前收盘价")
      .def_readonly("max_pull_back1", &PositionExtInfo::maxPullBack1,
                    "最大回撤百分比1(仅使用最大收盘价和最低收盘价计算)(负数)")
      .def_readonly("max_pull_back2", &PositionExtInfo::maxPullBack2,
                    "最大回撤百分比2(使用期间最高价最大值和最低价最小值计算)（负数）")
      .def_readonly("current_profit", &PositionExtInfo::currentProfit,
                    "当前盈亏(不含未发生的交易成本))")
      .def_property_readonly("current_pull_back1", &PositionExtInfo::currentPullBack1,
                             "当前回撤百分比1(仅使用最大收盘价和当前收盘价计算)")
      .def_property_readonly("current_pull_back2", &PositionExtInfo::currentPullBack2,
                             "当前回撤百分比2(使用期间最高价最大值和当前收盘价计算)")
      .def_property_readonly(
        "max_floating_Profit1", &PositionExtInfo::maxFloatingProfit1,
        "期间最大浮盈1 (正数, 仅使用收盘价计算, 不含预计卖出成本, 多次买卖时统计不准)")
      .def_property_readonly(
        "max_floating_profit2", &PositionExtInfo::maxFloatingProfit2,
        "期间最大浮盈2 (正数, 使用最高值最大值进行计算,不含预计卖出成本，多次买卖时统计不准)")
      .def_property_readonly(
        "min_loss_profit1", &PositionExtInfo::minLossProfit1,
        "期间最大浮亏1（负数，仅使用收盘价计算, 不含预计卖出成本，多次买卖时统计不准)")
      .def_property_readonly(
        "min_loss_profit2", &PositionExtInfo::minLossProfit2,
        "期间最大浮亏2（负数，仅期间最低价计算, 不含预计卖出成本，多次买卖时统计不准) ");

    m.def("get_max_pull_back", getMaxPullBack, py::arg("tm"), py::arg("date"),
          py::arg("ktype") = KQuery::DAY,
          R"(get_max_pull_back(tm, date, ktype=Query.DAY) -> price_t
    
    获取指定时刻时账户的最大回撤百分比（负数）

    :param TradeManager tm: 指定账户
    :param Datetime date: 指定日期（包含该时刻）
    :param Query.KType ktype: k线类型
    :return: 最大回撤百分比)");

    m.def(
      "get_position_ext_info_list", getPositionExtInfoList, py::arg("tm"), py::arg("current_time"),
      py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
      R"(get_position_ext_info_list(tm, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])
          
    获取账户最后交易时刻之后指定时间的持仓详情（未平常记录）
 
    :param TradeManager tm: 指定账户
    :param Datetime current_time: 当前时刻（需大于等于最后交易时刻）
    :param Query.KType ktype: k线类型
    :param int trade_mode: 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
    :return: 持仓扩展详情列表)");

    m.def(
      "get_history_position_ext_info_list", getHistoryPositionExtInfoList, py::arg("tm"),
      py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
      R"(get_history_position_ext_info_list(tm, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])
          
    获取账户历史持仓扩展详情（已平仓记录）
 
    :param TradeManager tm: 指定账户
    :param Query.KType ktype: k线类型
    :param int trade_mode: 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
    :return: 持仓扩展详情列表)");

    m.def("get_ext_performance", getExtPerformance, py::arg("tm"),
          py::arg("datetime") = Datetime::now(), py::arg("ktype") = KQuery::DAY,
          R"(get_ext_performance(tm, datetime=Datetime.now(), ktype=Query.DAY) -> Performance)

    获取账户指定时刻的账户扩展表现
 
    :param TradeManager tm: 账户
    :param Datetime datetime: 指定时刻
    :param Query.KType ktype: k线类型
    :return: 账户扩展表现)");
}