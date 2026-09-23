/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <csignal>
#include <hikyuu/strategy/Strategy.h>
#include <hikyuu/strategy/BrokerTradeManager.h>
#include <hikyuu/strategy/RunSystemInStrategy.h>
#include <hikyuu/strategy/RunMultiSystemInStrategy.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Strategy(py::module& m) {
    Datetime null_date;
    py::class_<Strategy, StrategyPtr>(m, "Strategy", py::dynamic_attr())
      .def(py::init<>())
      .def(
        py::init<const vector<string>&, const vector<KQuery::KType>&,
                 const unordered_map<string, int64_t>&, const std::string&, const std::string&>(),
        py::arg("code_list"), py::arg("ktype_list"),
        py::arg("preload_num") = unordered_map<string, int64_t>(), py::arg("name") = "Strategy",
        py::arg("config") = "", R"(Create the strategy runtime
           
    :param list code_list: the security code list, e.g.: ["sz000001", "sz000002"]; "all" means all the securities
    :param list ktype_list: the K-line type list, e.g.: ["day", "min"]
    :param dict preload_num: the number of the preloaded K-lines, e.g.: {"day_max": 1000, "min_max": 2000}
    :param str name: the strategy name
    :param str config: the configuration file name (if a standalone configuration file is needed; otherwise, when it is empty, the default hikyuu configuration file is used))")

      .def(py::init<const StrategyContext&, const string&, const string&>(), py::arg("context"),
           py::arg("name") = "Strategy", py::arg("config") = "",
           R"(Create the strategy runtime with a context

    :param StrategyContext context: the context instance
    :param str name: the strategy name
    :param str config: the configuration file name (if a standalone configuration file is needed; otherwise, when it is empty, the default hikyuu configuration file is used))")

      .def_property("name", py::overload_cast<>(&Strategy::name, py::const_),
                    py::overload_cast<const string&>(&Strategy::name),
                    py::return_value_policy::copy, "The strategy name")

      .def_property_readonly("running", &Strategy::running, "Get the current running state")
      .def_property_readonly("context", &Strategy::context, py::return_value_policy::copy,
                             "Get the strategy context")
      .def_property("tm", &Strategy::getTM, &Strategy::setTM, "The associated trade manager instance")
      .def_property("sp", &Strategy::getSP, &Strategy::setSP, "The slippage algorithm")
      .def_property_readonly("is_backtesting", &Strategy::isBacktesting, "The backtest state")

      .def(
        "start",
        [](Strategy& self, bool auto_recieve_spot) {
            // In python, before start, forcibly add an empty function, used to catch KeyboardInterrupt to terminate the strategy
            py::object func = py::eval("lambda stg: None");
            HKU_CHECK(check_pyfunction_arg_num(func, 1), "Number of parameters does not match!");
            auto new_func = [=](Strategy* stg) {
                try {
                    func(stg);
                } catch (py::error_already_set& e) {
                    if (e.matches(PyExc_KeyboardInterrupt)) {
                        printf("KeyboardInterrupt\n");
                        raise(SIGTERM);
                    } else {
                        HKU_ERROR(e.what());
                    }
                } catch (...) {
                    // do nothing
                }
            };
            self.runDaily(new_func, Seconds(1), "SH", true);

            self.start();
        },
        py::arg("auto_recieve_spot") = true, R"(start(self)

    Start the strategy execution; please execute it after completing the related callback settings.

    :param bool auto_recieve_spot: whether to receive the market data automatically)")

      .def(
        "on_change",
        [](Strategy& self, py::object func) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            HKU_CHECK(check_pyfunction_arg_num(func, 3), "Number of parameters does not match!");
            py::object c_func = func.attr("__call__");
            auto new_func = [=](Strategy* stg, const Stock& stk, const SpotRecord& spot) {
                try {
                    c_func(stg, stk, spot);
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
            self.onChange(new_func);
        },
        R"(onchang(self, func)
           
    Set the callback notification of the security data update

    :param func: a callable object such as an ordinary function, func(stg: Strategy, stock: Stock, spot: SpotRecord)")

      .def(
        "on_received_spot",
        [](Strategy& self, py::object func) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            HKU_CHECK(check_pyfunction_arg_num(func, 2), "Number of parameters does not match!");
            py::object c_func = func.attr("__call__");
            auto new_func = [=](Strategy* stg, Datetime revTime) {
                try {
                    c_func(stg, revTime);
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
            self.onReceivedSpot(new_func);
        },
        R"(on_received_spot(self, func)

    Set the callback notification of the security data update

    :param func: a callable object such as an ordinary function, func(stg: Strategy, revTime: Datetime))")

      .def(
        "run_daily",
        [](Strategy& self, py::object func, const TimeDelta& time, std::string market,
           bool ignore_market) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            HKU_CHECK(check_pyfunction_arg_num(func, 1), "Number of parameters does not match!");
            py::object c_func = func.attr("__call__");
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
            self.runDaily(new_func, time, market, ignore_market);
        },
        py::arg("func"), py::arg("time"), py::arg("market") = "SH",
        py::arg("ignore_market") = false, R"(run_daily(self, func)
        
    Set the callback executed in a loop within the day. If the market open/close is ignored, it loops from the start moment by the interval time,
    otherwise, at the first execution, the time interval is aligned with the market open time, and the execution stops in the non-trading time.

    :param func: a callable object such as an ordinary function, func(stg: Strategy)
    :param TimeDelta time: the interval time, e.g. an interval of 3 seconds: TimeDelta(0, 0, 0, 3) or Seconds(3)
    :param str market: which market's open/close times to use
    :param ignore_market: ignore the market open/close times)")

      .def(
        "run_daily_at",
        [](Strategy& self, py::object func, const TimeDelta& time, bool ignore_holiday) {
            HKU_CHECK(py::hasattr(func, "__call__"), "func is not callable!");
            HKU_CHECK(check_pyfunction_arg_num(func, 1), "Number of parameters does not match!");
            py::object c_func = func.attr("__call__");
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
            self.runDailyAt(new_func, time, ignore_holiday);
        },
        py::arg("func"), py::arg("time"), py::arg("ignore_holiday") = true,
        R"(run_daily_at(self, func)

    Set the callback executed at a fixed time every day

    :param func: a callable object such as an ordinary function, func(stg: Strategy)
    :param TimeDelta time: the execution moment, e.g. at 15 o'clock every day: TimeDelta(0, 15)
    :param ignore_holiday: do not execute on the holidays)")

      .def("today", &Strategy::today, R"(today(self)

    Get the current trading day date (use this method instead of Datetime.today(), so that the backtest and the live trading are consistent))")

      .def("now", &Strategy::now, R"(now(self)   

    Get the current time (use this method instead of Datetime.now(), so that the backtest and the live trading are consistent))")

      .def("next_datetime", &Strategy::nextDatetime, R"(next_datetime(self)

    The next trading time point (used for the backtest))")

      .def("get_current_price", &Strategy::getCurrentPrice, py::arg("stk"), py::arg("ktype"),
           "Get the current price; when invalid, return constant.null_price")

      .def("get_price_by_time", &Strategy::getPriceByTime, py::arg("stk"), py::arg("time"),
           py::arg("ktype") = KQuery::MIN,
           R"(get_price_by_time(self, stk, time, ktype)

    Get the price at the specified time point of the current day
    :param Stock stk: the stock object
    :param TimeDelta time: the specified time
    :param KQuery.KType ktype: the K-line type, defaulting to the minute line
    :return: the price; when invalid, return constant.null_price)")

      .def("get_kdata", &Strategy::getKData, py::arg("stk"), py::arg("start_date"),
           py::arg("end_date"), py::arg("ktype"), py::arg("recover_type") = KQuery::NO_RECOVER,
           R"(get_kdata(self, stk, start_date, end_date, ktype, recover_type)

    Get the K-line data of the specified security within the specified date range (to keep the live trading and the backtest consistent, please use this method to get the K-line data)
    :param Stock stk: the specified security
    :param Datetime start_date: the start date
    :param Datetime end_date: the end date
    :param KQuery.KType ktype: the K-line type
    :param KQuery.RecoverType recover_type: the recovery type
    :return: the K-line data
    :rtype: KData)")

      .def(
        "get_last_kdata",
        py::overload_cast<const Stock&, const Datetime&, const KQuery::KType&, KQuery::RecoverType>(
          &Strategy::getLastKData, py::const_),
        py::arg("stk"), py::arg("start_date"), py::arg("ktype"),
        py::arg("recover_type") = KQuery::NO_RECOVER)
      .def("get_last_kdata",
           py::overload_cast<const Stock&, size_t, const KQuery::KType&, KQuery::RecoverType>(
             &Strategy::getLastKData, py::const_),
           py::arg("stk"), py::arg("lastnum"), py::arg("ktype"),
           py::arg("recover_type") = KQuery::NO_RECOVER,
           R"(get_last_kdata(self, stk, start_date, ktype, recover_type)

    Get the K-line data of the specified security from the specified date to the current time (to keep the live trading and the backtest consistent, please use this method to get the K-line data)

    or get the last last_num K-line records that can currently be obtained (to keep the live trading and the backtest consistent, please use this method to get the K-line data)

    :param Stock stk: the specified security
    :param Datetime start_date: the start date  (or an int type, indicating how many trading days to go back from the current date)
    :param KQuery.KType ktype: the K-line type
    :param KQuery.RecoverType recover_type: the recovery type
    :return: the K-line data
    :rtype: KData)")

      .def("order", py::overload_cast<const Stock&, double, const string&>(&Strategy::order),
           py::arg("stock"), py::arg("num"), py::arg("remark") = "",
           R"(order(self, stock, num, remark='')

    Place an order by the quantity (a positive number is a buy, a negative number is a sell)
    :param Stock stock: the specified security
    :param int num: the order quantity
    :param str remark: the order remark)")

      .def("order_value",
           py::overload_cast<const Stock&, price_t, const string&>(&Strategy::orderValue),
           py::arg("stock"), py::arg("value"), py::arg("remark") = "",
           R"(order_value(self, stock, value, remark='')

    Place an order by the expected security market value, i.e. how much money of the security you want to buy (a positive number is a buy, a negative number is a sell)
    :param Stock stock: the specified security
    :param float value: the funds invested to buy
    :param str remark: the order remark)")

      .def(
        "buy",
        py::overload_cast<const Stock&, price_t, double, double, double, SystemPart, const string&>(
          &Strategy::buy),
        py::arg("stock"), py::arg("price"), py::arg("num"), py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("part") = SystemPart::PART_SIGNAL,
        py::arg("remark") = "")
      .def(
        "sell",
        py::overload_cast<const Stock&, price_t, double, double, double, SystemPart, const string&>(
          &Strategy::sell),
        py::arg("stock"), py::arg("price"), py::arg("num"), py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("part") = SystemPart::PART_SIGNAL,
        py::arg("remark") = "");

    m.def("crtBrokerTM", crtBrokerTM, py::arg("broker"), py::arg("cost_func") = TC_Zero(),
          py::arg("name") = "SYS", py::arg("other_brokers") = std::vector<OrderBrokerPtr>());

    m.def("run_in_strategy",
          py::overload_cast<const SYSPtr&, const Stock&, const KQuery&, const OrderBrokerPtr&,
                            const TradeCostPtr&, const std::vector<OrderBrokerPtr>&>(runInStrategy),
          py::arg("sys"), py::arg("stock"), py::arg("query"), py::arg("broker"),
          py::arg("cost_func"), py::arg("other_brokers") = std::vector<OrderBrokerPtr>(),
          R"(run_in_strategy(sys, stock, query, broker, cost_func, [other_brokers=[]])
          
    Execute the system trading SYS in the strategy runtime
    Currently only the systems with both buy_delay|sell_delay being false are supported, i.e. trading at the close
 
    :param sys: the trading system
    :param stock: the trading object
    :param query: the query condition
    :param broker: the order broker (dedicated to the order broker synchronizing with the account assets)
    :param cost_func: the cost function
    :param other_brokers: the other order brokers)");



    m.def("crt_sys_strategy", crtSysStrategy, py::arg("sys"), py::arg("stk_market_code"),
          py::arg("query"), py::arg("broker"), py::arg("cost_func"),
          py::arg("name") = "SYSStrategy",
          py::arg("other_brokers") = std::vector<OrderBrokerPtr>(), py::arg("config") = "");

    m.def("crt_multi_sys_strategy", crtMultiSysStrategy, py::arg("ms"), py::arg("stk_market_code"),
          py::arg("query"), py::arg("broker"), py::arg("cost_func"),
          py::arg("name") = "MultiSYSStrategy",
          py::arg("other_brokers") = std::vector<OrderBrokerPtr>(), py::arg("config") = "",
          R"(crt_multi_sys_strategy(ms, stk_market_code, query, broker, cost_func, [other_brokers=[]], [name='MultiSYSStrategy'], [config=''])

    Create the aggregate system strategy (the MultiSystem live trading entry).
    The parent account uses the BrokerTM synchronized with the broker, the sub-systems use their own shadow/virtual accounts (mode A/B is decided internally by MultiSystem).
    Currently only the sub-systems with both buy_delay | sell_delay being false are supported, i.e. the trade is executed at the close.

    :param ms: the aggregate trading system MultiSystem
    :param str stk_market_code: the driving instrument (e.g. 'SH000001', used to align the time axis)
    :param query: the query condition
    :param broker: the order broker (the order broker synchronized with the parent account assets)
    :param cost_func: the cost function
    :param other_brokers: the other order brokers
    :param str name: the strategy name
    :param str config: the config file
    :return: the strategy runtime instance
    :rtype: Strategy)");


}