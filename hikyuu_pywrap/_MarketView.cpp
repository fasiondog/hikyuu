/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <hikyuu/MarketView.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_MarketView(py::module& m) {
    m.def(
      "get_market_view",
      [](const py::sequence& stks, const Datetime& datetime, const KQuery::KType& ktype) {
          SPEND_TIME(get_market_view);
          StockList stks_list = python_list_to_vector<Stock>(stks);
          auto view = getMarketView(stks_list, datetime, ktype);

          size_t total = view.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // 创建数组
          py::array_t<int64_t> datetime_arr(total);
          py::array_t<double> open_arr(total);
          py::array_t<double> high_arr(total);
          py::array_t<double> low_arr(total);
          py::array_t<double> close_arr(total);
          py::array_t<double> amount_arr(total);
          py::array_t<double> vol_arr(total);

          // 获取缓冲区并填充数据
          auto datetime_buf = datetime_arr.request();
          auto open_buf = open_arr.request();
          auto high_buf = high_arr.request();
          auto low_buf = low_arr.request();
          auto close_buf = close_arr.request();
          auto amount_buf = amount_arr.request();
          auto vol_buf = vol_arr.request();

          int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
          double* open_ptr = static_cast<double*>(open_buf.ptr);
          double* high_ptr = static_cast<double*>(high_buf.ptr);
          double* low_ptr = static_cast<double*>(low_buf.ptr);
          double* close_ptr = static_cast<double*>(close_buf.ptr);
          double* amount_ptr = static_cast<double*>(amount_buf.ptr);
          double* vol_ptr = static_cast<double*>(vol_buf.ptr);

          auto pandas = py::module_::import("pandas");
          py::list code_list(total);
          py::list name_list(total);
          auto* ptr = view.data();
          for (size_t i = 0; i < total; i++) {
              const Stock& stock = ptr[i].stock;
              code_list[i] = stock.market_code();
              name_list[i] = stock.name();

              const KRecord& k = ptr[i].krecord;
              datetime_ptr[i] = k.datetime.timestamp() * 1000LL;
              open_ptr[i] = k.openPrice;
              high_ptr[i] = k.highPrice;
              low_ptr[i] = k.lowPrice;
              close_ptr[i] = k.closePrice;
              amount_ptr[i] = k.transAmount;
              vol_ptr[i] = k.transCount;
          }

          py::dict columns;
          columns["market_code"] = pandas.attr("Series")(code_list, py::arg("dtype") = "string");
          columns["name"] = pandas.attr("Series")(name_list, py::arg("dtype") = "string");
          columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
          columns["open"] = open_arr;
          columns["high"] = high_arr;
          columns["low"] = low_arr;
          columns["close"] = close_arr;
          columns["amount"] = amount_arr;
          columns["volume"] = vol_arr;

          return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      py::arg("stks"), py::arg("datetime") = Datetime::today(), py::arg("ktype") = KQuery::DAY);
}