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

// The pybind stl binding must come before the other type_caster

// Make the custom vector behave like a list in python
// The vectors of the simple types are no longer exported, only the complex structList is exported,
// avoiding the performance impact
// PYBIND11_MAKE_OPAQUE(StringList);
// PYBIND11_MAKE_OPAQUE(PriceList);
PYBIND11_MAKE_OPAQUE(DatetimeList);
PYBIND11_MAKE_OPAQUE(KRecordList);
// PYBIND11_MAKE_OPAQUE(StockList); // The StockList is not large, so letting pybind convert it from
// and to a list automatically is more convenient
PYBIND11_MAKE_OPAQUE(StockWeightList);
// PYBIND11_MAKE_OPAQUE(IndicatorList); // It cannot be compiled
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