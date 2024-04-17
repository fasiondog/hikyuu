/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20231231 added by fasiondog
 */

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <hikyuu/hikyuu.h>

using namespace hku;

// pybind stl 绑定必须在其他 type_caster 之前

// 让自定义的 vector 在 python 中表现的像 list 一样
// 简单类型的 vector 不再导出，只导出复杂的 structList，避免影响性能
// PYBIND11_MAKE_OPAQUE(StringList);
// PYBIND11_MAKE_OPAQUE(PriceList);
PYBIND11_MAKE_OPAQUE(DatetimeList);
PYBIND11_MAKE_OPAQUE(KRecordList);
// PYBIND11_MAKE_OPAQUE(StockList); // StockList 数据量不大，让 pybind 自动从 list 互转比较方便
PYBIND11_MAKE_OPAQUE(StockWeightList);
// PYBIND11_MAKE_OPAQUE(IndicatorList); // 无法编译
PYBIND11_MAKE_OPAQUE(TimeLineList);
PYBIND11_MAKE_OPAQUE(TransList);
// PYBIND11_MAKE_OPAQUE(BorrowRecordList);
// PYBIND11_MAKE_OPAQUE(LoanRecordList);
PYBIND11_MAKE_OPAQUE(PositionRecordList);
// PYBIND11_MAKE_OPAQUE(FundsList);
PYBIND11_MAKE_OPAQUE(TradeRecordList);
PYBIND11_MAKE_OPAQUE(SystemWeightList);
// PYBIND11_MAKE_OPAQUE(SystemList);
PYBIND11_MAKE_OPAQUE(ScoreRecordList);