/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <arrow/python/pyarrow.h>
#include <hikyuu/views/arrow_views.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_arrow_views(py::module& m) {
    m.def(
      "get_market_view",
      [](const py::sequence& stks, const Datetime& date, const string& market) {
          StockList stks_list = python_list_to_vector<Stock>(stks);
          arrow::Result<std::shared_ptr<arrow::Table>> view;
          if (date.isNull()) {
              view = getMarketView(stks_list, market);
          } else {
              view = getMarketView(stks_list, date, market);
          }
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_borrow<py::object>(raw_obj);
          return t.attr("to_pandas")();
      },
      py::arg("stks"), py::arg("date") = Datetime(), py::arg("market") = "SH",
      R"(get_market_view(stks[, date=Datetime(), market='SH']) -> pandas.DataFrame

    获取指定股票集合在指定交易日的行情数据，不包含当日停牌无数据的股票。如未指定日期，则返回最后交易日行情数据，
    如同时接收了行情数据，则为实时行情。

    注: 此函数依赖于日线数据
    
    :param list[Stock] stks: 股票列表
    :param Datetime date: 获取指定日期的行情数据
    :param str market: 市场代码
    :return: 指定股票列表最后行情数据
    :rtype: pandas.DataFrame)");

    m.def(
      "get_inds_view",
      [](const StockList& stks, const IndicatorList& inds, const KQuery& query,
         const string& market) {
          SPEND_TIME(get_inds_view);
          auto view = getIndicatorsView(stks, inds, query, market);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_borrow<py::object>(raw_obj);
          return t.attr("to_pandas")();
      },
      py::arg("stks"), py::arg("inds"), py::arg("query"), py::arg("market") = "SH");

    m.def(
      "get_inds_view",
      [](const StockList& stks, const IndicatorList& inds, const Datetime& date, size_t cal_len,
         const KQuery::KType& ktype, const string& market) {
          SPEND_TIME(get_inds_view);
          auto view = getIndicatorsView(stks, inds, date, cal_len, ktype, market);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_borrow<py::object>(raw_obj);
          return t.attr("to_pandas")();
      },
      py::arg("stks"), py::arg("inds"), py::arg("date"), py::arg("cal_len") = 100,
      py::arg("ktype") = KQuery::DAY, py::arg("market") = "SH");

    m.def(
      "krecords_to_pa",
      [](const KRecordList& ks) {
          SPEND_TIME(krecords_to_pa);
          auto view = getKRecordListView(ks);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将KRecordList转换为parraw.Table");

    m.def(
      "timeline_to_pa",
      [](const TimeLineList& ts) {
          auto view = getTimeLineListView(ts);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将分时线记录转换为 pyarrow.Table 对象");

    m.def(
      "translist_to_pa",
      [](const TransRecordList& ts) {
          auto view = getTransRecordListView(ts);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将分笔记录转换为 pyarrow.Table 对象");

    m.def(
      "weights_to_pa",
      [](const StockWeightList& sws) {
          auto view = getStockWeightListView(sws);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将权息记录列表转换为 pyarrow.Table 对象");

    m.def(
      "dates_to_pa",
      [](const DatetimeList& dates) {
          auto view = getDatetimeListView(dates);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将日期列表转换为 pyarrow.Table 对象");

    m.def(
      "trades_to_df",
      [](const TradeRecordList& trades) {
          auto view = getTradeRecordListView(trades);
          HKU_ARROW_TABLE_CHECK(view);
          arrow::py::import_pyarrow();
          PyObject* raw_obj = arrow::py::wrap_table(*view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_borrow<py::object>(raw_obj);
      },
      "将交易记录列表转换为 pyarrow.Table 对象");
}