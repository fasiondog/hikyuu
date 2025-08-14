/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <arrow/python/pyarrow.h>
#include <hikyuu/view/MarketView.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_MarketView(py::module& m) {
    m.def(
      "get_market_view",
      [](const py::sequence& stks, const Datetime& date, const string& market) {
          StockList stks_list = python_list_to_vector<Stock>(stks);
          std::shared_ptr<arrow::Table> view;
          if (date.isNull()) {
              view = getMarketView(stks_list, market);
          } else {
              view = getMarketView(stks_list, date, market);
          }
          HKU_ASSERT(view);

          arrow::py::import_pyarrow();

          // 使用Arrow的Python绑定将Table转换为Python对象
          PyObject* raw_obj = arrow::py::wrap_table(view);
          if (raw_obj == nullptr) {
              throw std::runtime_error("wrap_table返回空指针");
          }

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
}