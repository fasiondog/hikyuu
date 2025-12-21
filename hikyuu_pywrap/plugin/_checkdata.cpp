/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-12-20
 *      Author: fasiondog
 */

#include <hikyuu/plugin/checkdata.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_checkdata(py::module& m) {
    m.def(
      "check_data",
      [](const py::sequence& stock_list, Datetime start_date, Datetime end_date,
         const KQuery::KType& ktype) {
          StockList stk_list;
          if (py::isinstance<StockManager>(stock_list)) {
              stk_list = StockManager::instance().getStockList();
          } else if (py::isinstance<Block>(stock_list)) {
              stk_list = stock_list.cast<Block>().getStockList();
          } else {
              stk_list = python_list_to_vector<Stock>(stock_list);
          }
          auto [x, y] = checkData(stk_list, start_date, end_date, ktype);
          py::module_ json_m = py::module_::import("json");
          py::object x_dict = json_m.attr("loads")(x);
          return py::make_tuple(x_dict, y);
      },
      "检查数据", py::arg("stock_list"), py::arg("start_date"), py::arg("end_date"),
      py::arg("ktype"));
}