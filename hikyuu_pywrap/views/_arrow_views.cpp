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
          StockList stks_list;
          if (py::isinstance<StockManager>(stks)) {
              stks_list = StockManager::instance().getStockList();
          } else if (py::isinstance<Block>(stks)) {
              stks_list = stks.cast<Block&>().getStockList();
          } else {
              stks_list = python_list_to_vector<Stock>(stks);
          }
          std::shared_ptr<arrow::Table> view;
          if (date.isNull()) {
              view = getMarketView(stks_list, market);
          } else {
              view = getMarketView(stks_list, date, market);
          }
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_steal<py::object>(raw_obj);
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
      [](const py::sequence& stks, const IndicatorList& inds, const KQuery& query,
         const string& market, bool parallel) {
          StockList stock_list;
          if (py::isinstance<StockManager>(stks)) {
              stock_list = StockManager::instance().getStockList();
          } else if (py::isinstance<Block>(stks)) {
              stock_list = stks.cast<Block>().getStockList();
          } else {
              stock_list = python_list_to_vector<Stock>(stks);
          }
          auto view = getIndicatorsView(stock_list, inds, query, market, parallel);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_steal<py::object>(raw_obj);
          return t.attr("to_pandas")();
      },
      py::arg("stks"), py::arg("inds"), py::arg("query"), py::arg("market") = "SH",
      py::arg("parallel") = false);

    m.def(
      "get_inds_view",
      [](const py::sequence& stks, const IndicatorList& inds, const Datetime& date, size_t cal_len,
         const KQuery::KType& ktype, const string& market, bool parallel) {
          StockList stks_list;
          if (py::isinstance<StockManager>(stks)) {
              stks_list = StockManager::instance().getStockList();
          } else if (py::isinstance<Block>(stks)) {
              stks_list = stks.cast<Block&>().getStockList();
          } else {
              stks_list = python_list_to_vector<Stock>(stks);
          }
          auto view = getIndicatorsView(stks_list, inds, date, cal_len, ktype, market, parallel);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          auto t = py::reinterpret_steal<py::object>(raw_obj);
          return t.attr("to_pandas")();
      },
      py::arg("stks"), py::arg("inds"), py::arg("date"), py::arg("cal_len") = 100,
      py::arg("ktype") = KQuery::DAY, py::arg("market") = "SH", py::arg("parallel") = false,
      R"(get_inds_view(stks, inds, date[, cal_len=100, ktype=Query.DAY, market='SH']) -> pandas.DataFrame)
    
    方式1: 获取指定日期的各证券的各指标结果

      :param stks: 证券列表
      :param list[Indicator] inds: 指标列表
      :param Datetime date: 指定日期
      :param int cal_len: 计算需要的数据长度
      :param str ktype: k线类型
      :param str market: 指定行情市场（用于日期对齐）
      :param bool parallel: 是否并行计算

    方式2: 获取按指定Query查询计算的各证券的各指标结果, 结果中将包含指定 Query 包含的所有指定市场交易日日期

      :param stks: 指定证券列表
      :param list[Indicator] inds: 指定指标列表
      :param Query query: 查询条件
      :param str market: 指定行情市场（用于日期对齐）
      :param bool parallel: 是否并行计算)");

    m.def(
      "krecords_to_pa",
      [](const KRecordList& ks) {
          auto view = getKRecordListView(ks);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将KRecordList转换为parraw.Table");

    m.def(
      "timeline_to_pa",
      [](const TimeLineList& ts) {
          auto view = getTimeLineListView(ts);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将分时线记录转换为 pyarrow.Table 对象");

    m.def(
      "translist_to_pa",
      [](const TransRecordList& ts) {
          auto view = getTransRecordListView(ts);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将分笔记录转换为 pyarrow.Table 对象");

    m.def(
      "weights_to_pa",
      [](const StockWeightList& sws) {
          auto view = getStockWeightListView(sws);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将权息记录列表转换为 pyarrow.Table 对象");

    m.def(
      "dates_to_pa",
      [](const DatetimeList& dates) {
          auto view = getDatetimeListView(dates);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将日期列表转换为 pyarrow.Table 对象");

    m.def(
      "trades_to_pa",
      [](const TradeRecordList& trades) {
          auto view = getTradeRecordListView(trades);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将交易记录列表转换为 pyarrow.Table 对象");

    m.def(
      "positions_to_pa",
      [](const PositionRecordList& positions) {
          auto view = getPositionRecordListView(positions);
          HKU_ASSERT(view);
          PyObject* raw_obj = arrow::py::wrap_table(view);
          HKU_CHECK(raw_obj, "Failed to wrap table to pyobject!");
          return py::reinterpret_steal<py::object>(raw_obj);
      },
      "将交易记录列表转换为 pyarrow.Table 对象");
}