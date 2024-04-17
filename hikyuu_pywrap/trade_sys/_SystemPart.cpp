/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-12
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>
#include <hikyuu/trade_sys/system/SystemPart.h>

using namespace hku;
namespace py = pybind11;

void export_SystemPart(py::module& m) {
    py::enum_<SystemPart>(m, "SystemPart", "系统关联部件（各自策略）枚举定义，用于修改相关部件参数")
      .value("ENVIRONMENT", PART_ENVIRONMENT, "外部环境")
      .value("CONDITION", PART_CONDITION, "系统前提条件")
      .value("SIGNAL", PART_SIGNAL, "信号产生器")
      .value("STOPLOSS", PART_STOPLOSS, "止损策略")
      .value("TAKEPROFIT", PART_TAKEPROFIT, "止赢策略")
      .value("MONEYMANAGER", PART_MONEYMANAGER, "资金管理策略")
      .value("PROFITGOAL", PART_PROFITGOAL, "盈利目标策略")
      .value("SLIPPAGE", PART_SLIPPAGE, "移滑价差算法")
      .value("ALLOCATEFUNDS", PART_ALLOCATEFUNDS, "资产分配算法")
      .value("INVALID", PART_INVALID, "无效系统部件")

      // 支持简写
      .value("EV", PART_ENVIRONMENT, "外部环境")
      .value("CN", PART_CONDITION, "系统前提条件")
      .value("SG", PART_SIGNAL, "信号产生器")
      .value("ST", PART_STOPLOSS, "止损策略")
      .value("TP", PART_TAKEPROFIT, "止赢策略")
      .value("MM", PART_MONEYMANAGER, "资金管理策略")
      .value("PG", PART_PROFITGOAL, "盈利目标策略")
      .value("SP", PART_SLIPPAGE, "移滑价差算法")
      .value("AF", PART_ALLOCATEFUNDS, "资产分配算法");
}