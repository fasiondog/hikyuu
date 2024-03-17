/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20231231 added by fasiondog
 */

#include <hikyuu/hikyuu.h>
#include "pybind_utils.h"
#include "bind_stl.h"

using namespace hku;
namespace py = pybind11;

void export_bind_stl(py::module& m) {
    // py::bind_vector<PriceList>(m, "PriceList");
    // py::bind_vector<StringList>(m, "StringList");
    py::bind_vector<DatetimeList>(m, "DatetimeList");
    py::bind_vector<KRecordList>(m, "KRecordList");
    py::bind_vector<StockList>(m, "StockList");
    py::bind_vector<StockWeightList>(m, "StockWeightList");
    // py::bind_vector<IndicatorList>(m, "Indicatorist");
    py::bind_vector<TimeLineList>(m, "TimeLineList");
    py::bind_vector<TransList>(m, "TransList");
    py::bind_vector<BorrowRecordList>(m, "BorrowRecordList");
    py::bind_vector<LoanRecordList>(m, "LoanRecordList");
    py::bind_vector<PositionRecordList>(m, "PositionRecordList");
    py::bind_vector<TradeRecordList>(m, "TradeRecordList");
    py::bind_vector<SystemWeightList>(m, "SystemWeightList");
    py::bind_vector<SystemList>(m, "SystemList");
    py::bind_vector<ScoreRecordList>(m, "ScoreRecordList");
}