/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <csignal>
#include <hikyuu/strategy/backtest.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_backtest(py::module& m) {
    Datetime null_date;
    m.def(
      "backtest",
      [](const StrategyContext& context, py::object on_bar, const TradeManagerPtr& tm,
         const Datetime& start_date, const Datetime& end_date, const KQuery::KType& ktype,
         const string& ref_market, int mode) {
          HKU_CHECK(py::hasattr(on_bar, "__call__"), "func is not callable!");
          HKU_CHECK(check_pyfunction_arg_num(on_bar, 1), "Number of parameters does not match!");
          py::object c_func = on_bar.attr("__call__");
          auto new_func = [=](Strategy* stg) {
              try {
                  c_func(stg);
              } catch (py::error_already_set& e) {
                  if (e.matches(PyExc_KeyboardInterrupt)) {
                      printf("KeyboardInterrupt\n");
                      raise(SIGTERM);
                  } else {
                      HKU_ERROR(e.what());
                  }
              } catch (const std::exception& e) {
                  HKU_ERROR(e.what());
              } catch (...) {
                  HKU_ERROR("Unknown error!");
              }
          };
          backtest(context, new_func, tm, start_date, end_date, ktype, ref_market, mode);
      },
      py::arg("context"), py::arg("on_bar"), py::arg("tm"), py::arg("start_date"),
      py::arg("end_date") = null_date, py::arg("ktype") = KQuery::DAY, py::arg("ref_market") = "SH",
      py::arg("mode") = 0);
}