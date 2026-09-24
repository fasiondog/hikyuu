/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <csignal>
#include <hikyuu/plugin/backtest.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_backtest(py::module& m) {
    Datetime null_date;
    m.def(
      "backtest",
      [](const StrategyContext& context, py::object on_bar, const TradeManagerPtr& tm,
         const Datetime& start_date, const Datetime& end_date, const KQuery::KType& ktype,
         const string& ref_market, int mode, bool support_short, SlippagePtr sp) {
          HKU_CHECK(py::hasattr(on_bar, "__call__"), "{}", htr("on_bar is not callable!"));
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
          backtest(context, new_func, tm, start_date, end_date, ktype, ref_market, mode,
                   support_short, sp);
      },
      py::arg("context"), py::arg("on_bar"), py::arg("tm"), py::arg("start_date"),
      py::arg("end_date") = null_date, py::arg("ktype") = KQuery::DAY, py::arg("ref_market") = "SH",
      py::arg("mode") = 0, py::arg("support_short") = false, py::arg("sp") = SlippagePtr());

    m.def(
      "backtest",
      [](py::object on_bar, const TradeManagerPtr& tm, const Datetime& start_date,
         const Datetime& end_date, const KQuery::KType& ktype, const string& ref_market, int mode,
         bool support_short, SlippagePtr sp) {
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
          backtest(new_func, tm, start_date, end_date, ktype, ref_market, mode, support_short, sp);
      },
      py::arg("on_bar"), py::arg("tm"), py::arg("start_date"), py::arg("end_date") = null_date,
      py::arg("ktype") = KQuery::DAY, py::arg("ref_market") = "SH", py::arg("mode") = 0,
      py::arg("support_short") = false, py::arg("sp") = SlippagePtr(),
      R"(backtest([context], on_bar, tm, start_date, end_date, ktype, ref_market, mode)

    The event-driven backtest, usually directly testing the main functions in the Strategy

    If hikyuu has already loaded the data, the context parameter can be ignored. Otherwise, similar to the Strategy, you need to pass in the context parameter actively,
    and the context contains the stock codes, the K-line types, the number of the K-lines, the K-line start dates, etc. that need to be loaded.
      
    :param StrategyContext context: the strategy context ()
    :param func on_bar: the strategy main execution function, e.g.: on_bar(stg: Strategy)
    :param TradeManager tm: the strategy test account
    :param Datetime start_date: the start date
    :param Datetime end_date: the end date (exclusive)
    :param Query.KType ktype: the K-line type (executing the test Bar by Bar according to this type)
    :param str ref_market: the market it belongs to
    :param mode the mode  0: execute the buy/sell operations at the close price of the current bar; 1: execute the buy/sell operations at the open price of the next bar
    :param support_short: whether short selling is supported
    :param Slippage sp: the slippage algorithm)");
}