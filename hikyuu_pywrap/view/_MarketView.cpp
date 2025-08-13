/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include <hikyuu/view/MarketView.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_MarketView(py::module& m) {
    m.def(
      "get_market_view",
      [](const py::sequence& stks, const Datetime& date, const string& market) {
          SPEND_TIME(get_market_view);
          StockList stks_list = python_list_to_vector<Stock>(stks);
          MarketView view;
          if (date.isNull()) {
              view = getMarketView(stks_list, market);
          } else {
              view = getMarketView(stks_list, date, market);
          }

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
          py::array_t<double> yesterday_close_arr(total);
          py::array_t<double> price_change_arr(total);
          py::array_t<double> amplitude_arr(total);
          py::array_t<double> turnover_arr(total);
          py::array_t<double> total_market_cap_arr(total);
          py::array_t<double> circulating_market_cap_arr(total);
          py::array_t<double> pb_arr(total);
          py::array_t<double> pe_arr(total);

          // 获取缓冲区并填充数据
          auto datetime_buf = datetime_arr.request();
          auto open_buf = open_arr.request();
          auto high_buf = high_arr.request();
          auto low_buf = low_arr.request();
          auto close_buf = close_arr.request();
          auto amount_buf = amount_arr.request();
          auto vol_buf = vol_arr.request();
          auto yesterday_close_buf = yesterday_close_arr.request();
          auto price_change_buf = price_change_arr.request();
          auto amplitude_buf = amplitude_arr.request();
          auto turnover_buf = turnover_arr.request();
          auto total_market_cap_buf = total_market_cap_arr.request();
          auto circulating_market_cap_buf = circulating_market_cap_arr.request();
          auto pb_buf = pb_arr.request();
          auto pe_buf = pe_arr.request();

          int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
          double* open_ptr = static_cast<double*>(open_buf.ptr);
          double* high_ptr = static_cast<double*>(high_buf.ptr);
          double* low_ptr = static_cast<double*>(low_buf.ptr);
          double* close_ptr = static_cast<double*>(close_buf.ptr);
          double* amount_ptr = static_cast<double*>(amount_buf.ptr);
          double* vol_ptr = static_cast<double*>(vol_buf.ptr);
          double* yesterday_close_ptr = static_cast<double*>(yesterday_close_buf.ptr);
          double* price_change_ptr = static_cast<double*>(price_change_buf.ptr);
          double* amplitude_ptr = static_cast<double*>(amplitude_buf.ptr);
          double* turnover_ptr = static_cast<double*>(turnover_buf.ptr);
          double* total_market_cap_ptr = static_cast<double*>(total_market_cap_buf.ptr);
          double* circulating_market_cap_ptr = static_cast<double*>(circulating_market_cap_buf.ptr);
          double* pb_ptr = static_cast<double*>(pb_buf.ptr);
          double* pe_ptr = static_cast<double*>(pe_buf.ptr);

          auto pandas = py::module_::import("pandas");
          py::list code_list(total);
          py::list name_list(total);
          auto* ptr = view.data();
          for (size_t i = 0; i < total; i++) {
              const Stock& stock = ptr[i].stock;
              code_list[i] = stock.market_code();
              name_list[i] = stock.name();

              datetime_ptr[i] = ptr[i].date.timestamp() * 1000LL;
              open_ptr[i] = ptr[i].open;
              high_ptr[i] = ptr[i].high;
              low_ptr[i] = ptr[i].low;
              close_ptr[i] = ptr[i].close;
              amount_ptr[i] = ptr[i].amount;
              vol_ptr[i] = ptr[i].volume;
              yesterday_close_ptr[i] = ptr[i].yesterday_close;
              price_change_ptr[i] = ptr[i].price_change;
              amplitude_ptr[i] = ptr[i].amplitude;
              turnover_ptr[i] = ptr[i].turnover;
              total_market_cap_ptr[i] = ptr[i].total_market_cap;
              circulating_market_cap_ptr[i] = ptr[i].circulating_market_cap;
              pb_ptr[i] = ptr[i].pb;
              pe_ptr[i] = ptr[i].pe;
          }

          py::dict columns;
          columns[htr("market_code").c_str()] =
            pandas.attr("Series")(code_list, py::arg("dtype") = "string");
          columns[htr("name").c_str()] =
            pandas.attr("Series")(name_list, py::arg("dtype") = "string");
          columns[htr("date").c_str()] = datetime_arr.attr("astype")("datetime64[ns]");
          columns[htr("open").c_str()] = open_arr;
          columns[htr("high").c_str()] = high_arr;
          columns[htr("low").c_str()] = low_arr;
          columns[htr("close").c_str()] = close_arr;
          columns[htr("amount").c_str()] = amount_arr;
          columns[htr("volume").c_str()] = vol_arr;
          columns[htr("yesterday_close").c_str()] = yesterday_close_arr;
          columns[htr("price_change(%)").c_str()] = price_change_arr;
          columns[htr("amplitude(%)").c_str()] = amplitude_arr;
          columns[htr("turnover(%)").c_str()] = turnover_arr;
          columns[htr("total_market_cap(W)").c_str()] = total_market_cap_arr;
          columns[htr("circulating_market_cap(W)").c_str()] = circulating_market_cap_arr;
          columns[htr("pb").c_str()] = pb_arr;
          columns[htr("pe").c_str()] = pe_arr;

          return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      py::arg("stks"), py::arg("date") = Datetime(), py::arg("market") = "SH",
      R"(get_market_view(stks[, date=Datetime(), market='SH']) -> pandas.DataFrame

    获取指定股票集合在指定交易日的行情数据，不包含当日停牌无数据的股票。如未指定日期，则返回最后交易日行情数据，
    如同时接收了行情数据，则为实时行情。
    
    :param list[Stock] stks: 股票列表
    :param Datetime date: 获取指定日期的行情数据
    :param str market: 市场代码
    :return: 指定股票列表最后行情数据
    :rtype: pandas.DataFrame)");
}