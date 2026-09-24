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
    py::enum_<SystemPart>(m, "SystemPart",
                          "The enumeration definition of the system related parts (the individual "
                          "strategies), used to modify the related part parameters")
      .value("ENVIRONMENT", PART_ENVIRONMENT, "The external environment")
      .value("CONDITION", PART_CONDITION, "The system precondition")
      .value("SIGNAL", PART_SIGNAL, "The signal generator")
      .value("STOPLOSS", PART_STOPLOSS, "The stop-loss strategy")
      .value("TAKEPROFIT", PART_TAKEPROFIT, "The take-profit strategy")
      .value("MONEYMANAGER", PART_MONEYMANAGER, "The money management strategy")
      .value("PROFITGOAL", PART_PROFITGOAL, "The profit goal strategy")
      .value("SLIPPAGE", PART_SLIPPAGE, "The slippage algorithm")
      .value("ALLOCATEFUNDS", PART_ALLOCATEFUNDS, "The asset allocation algorithm")
      .value("SYS", PART_SYSTEM, "The aggregate sub-system (added by the recursive combination refactoring)")
      .value("INVALID", PART_INVALID, "An invalid system part")

      // Support the abbreviations
      .value("EV", PART_ENVIRONMENT, "The external environment")
      .value("CN", PART_CONDITION, "The system precondition")
      .value("SG", PART_SIGNAL, "The signal generator")
      .value("ST", PART_STOPLOSS, "The stop-loss strategy")
      .value("TP", PART_TAKEPROFIT, "The take-profit strategy")
      .value("MM", PART_MONEYMANAGER, "The money management strategy")
      .value("PG", PART_PROFITGOAL, "The profit goal strategy")
      .value("SP", PART_SLIPPAGE, "The slippage algorithm")
      .value("AF", PART_ALLOCATEFUNDS, "The asset allocation algorithm");
}