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

    m.def(
      "backtest",
      [](py::object on_bar, const TradeManagerPtr& tm, const Datetime& start_date,
         const Datetime& end_date, const KQuery::KType& ktype, const string& ref_market, int mode) {
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
          backtest(new_func, tm, start_date, end_date, ktype, ref_market, mode);
      },
      py::arg("on_bar"), py::arg("tm"), py::arg("start_date"), py::arg("end_date") = null_date,
      py::arg("ktype") = KQuery::DAY, py::arg("ref_market") = "SH", py::arg("mode") = 0,
      R"(backtest([context], on_bar, tm, start_date, end_date, ktype, ref_market, mode)

    事件驱动式回测, 通常直接测试 Strategy 中的主体函数

    如果 hikyuu 已经加载数据，可以忽略 context 参数。否则通 Strategy 类似，需要主动传入 context 参数，
    context 中包含需要加载的股票代码、K线类型、K线数量、K线起始日期等信息。
      
    :param StrategyContext context: 策略上下文 ()
    :param func on_bar: 策略主体执行函数, 如: on_bar(stg: Strategy)
    :param TradeManager tm: 策略测试账户
    :param Datetime start_date: 起始日期
    :param Datetime end_date: 结束日期（不包含其本身）
    :param Query.KType ktype: K线类型(按该类型逐 Bar 执行测试)
    :param str ref_market: 所属市场
    :param mode 模式  0: 当前bar收盘价执行买卖操作; 1: 下一bar开盘价执行买卖操作)");
}