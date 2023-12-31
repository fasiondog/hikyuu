/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20231231 added by fasiondog
 */

#pragma once

#include <pybind11/pybind11.h>
#include <hikyuu/hikyuu.h>

using namespace hku;

// pybind stl 绑定必须在其他 type_caster 之前

// 简单类型的 vector 不再导出，只导出复杂的 struct(影响性能)
// PYBIND11_MAKE_OPAQUE(StringList);
// PYBIND11_MAKE_OPAQUE(PriceList);
PYBIND11_MAKE_OPAQUE(DatetimeList);
PYBIND11_MAKE_OPAQUE(KRecordList);
PYBIND11_MAKE_OPAQUE(StockWeightList);
PYBIND11_MAKE_OPAQUE(TimeLineList);
PYBIND11_MAKE_OPAQUE(TransList);
PYBIND11_MAKE_OPAQUE(BorrowRecordList);
PYBIND11_MAKE_OPAQUE(LoanRecordList);
PYBIND11_MAKE_OPAQUE(PositionRecordList);
PYBIND11_MAKE_OPAQUE(TradeRecordList);
PYBIND11_MAKE_OPAQUE(SystemWeightList);
PYBIND11_MAKE_OPAQUE(SystemList);