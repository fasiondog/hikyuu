/*
 * _Portfolio.cpp
 *
 *  Created on: 2016-03-29
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/portfolio/imp/SimplePortfolio.h>
#include <hikyuu/trade_sys/portfolio/imp/WithoutAFPortfolio.h>
#include "_Portfolio.h"
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

PyPortfolio::PyPortfolio(const Portfolio& base) : Portfolio(base) {}

PyPortfolio::~PyPortfolio() {}

string PyPortfolio::str() const {
    PYBIND11_OVERLOAD(string, PyPortfolio, str);
}

void PyPortfolio::_reset() {
    PYBIND11_OVERLOAD(void, PyPortfolio, _reset);
}

void PyPortfolio::_readyForRun() {
    PYBIND11_OVERLOAD(void, PyPortfolio, _readyForRun);
}

void PyPortfolio::_runMomentOnOpen(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    PYBIND11_OVERLOAD(void, PyPortfolio, _runMomentOnOpen, date, nextCycle, adjust);
}

void PyPortfolio::_runMomentOnClose(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    PYBIND11_OVERLOAD(void, PyPortfolio, _runMomentOnClose, date, nextCycle, adjust);
}

json PyPortfolio::lastSuggestion() const {
    PYBIND11_OVERLOAD(json, PyPortfolio, lastSuggestion);
}

void PyPortfolio::set_tm(py::object tm) {
    py::gil_scoped_acquire gil;
    auto tmp_tm = tm;
    setTM(tm.cast<TradeManagerPtr>());
    tmp_tm.release();
}

void PyPortfolio::set_se(py::object se) {
    py::gil_scoped_acquire gil;
    auto tmp_se = se;
    setSE(se.cast<SelectorPtr>());
    tmp_se.release();
}

void PyPortfolio::set_af(py::object af) {
    py::gil_scoped_acquire gil;
    auto tmp_af = af;
    setAF(af.cast<AFPtr>());
    tmp_af.release();
}

void export_Portfolio(py::module& m) {
    py::class_<Portfolio, PortfolioPtr, PyPortfolio>(m, "Portfolio", py::dynamic_attr(),
                                                     R"(Implements a portfolio of multiple targets and multiple strategies)")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const string&, const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())

      .def("__str__", &Portfolio::str)
      .def("__repr__", &Portfolio::str)

      .def_property("name", py::overload_cast<>(&Portfolio::name, py::const_),
                    py::overload_cast<const string&>(&Portfolio::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &Portfolio::getQuery, &Portfolio::setQuery,
                    py::return_value_policy::copy, "The query condition")

      .def_property(
        "tm", &Portfolio::getTM, [](PyPortfolio& self, py::object tm) { self.set_tm(tm); },
        "Set or get the trade manager object")
      .def_property(
        "se", &Portfolio::getSE, [](PyPortfolio& self, py::object se) { self.set_se(se); },
        "Set or get the trading object selection algorithm")
      .def_property(
        "af", &Portfolio::getAF, [](PyPortfolio& self, py::object af) { self.set_af(af); },
        "Set or get the asset allocation algorithm")
      .def_property_readonly("real_sys_list", &Portfolio::getRealSystemList,
                             py::return_value_policy::copy, "The actual running system list set by the PF at runtime")

      .def("get_param", &Portfolio::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (Portfolio::*)(const std::string&, const boost::any&)>(
             &Portfolio::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &Portfolio::haveParam, "Whether the specified parameter exists")

      .def("reset", &Portfolio::reset, "The reset operation")
      .def("clone", &Portfolio::clone, "The clone operation")

      .def("get_running_dates", &Portfolio::getRunningDates, "Get the running date list")
      .def("get_adjust_dates", &Portfolio::getAdjustDates, "Get the position adjustment date list")
      .def("get_cycle_end_dates", &Portfolio::getCycleEndDates, "Get the list of the position adjustment period end dates")
      .def(
        "get_adjust_turnover",
        [](const Portfolio& pf) {
            const auto& turnover = pf.getAdjustTurnover();
            py::list ret;
            for (const auto& item : turnover) {
                ret.append(py::make_tuple(item.first, item.second));
            }
            return ret;
        },
        "Get the list of the position adjustment turnover rates")

      .def("run", &Portfolio::run, py::arg("query"), py::arg("force") = false,
           R"(run(self, query[, force=false])
    
    Run the portfolio strategy. When the query condition and the parts have not changed, the PF will not actually calculate by default when it is executed the second time.
    However, since the parameters of the parts may change, whether a recalculation is needed cannot be judged automatically; you can specify a forced calculation manually.
        
    :param Query query: the query condition
    :param bool force: force recalculating)")

      .def(
        "last_suggestion",
        [](const Portfolio& pf) {
            json j = pf.lastSuggestion();
            std::string json_str = j.dump();
            py::module json_module = py::module::import("json");
            return json_module.attr("loads")(json_str);
        },
        "After the backtest is completed, return the trade records of the last day, and the delayed buy and sell requests that need to be delayed")

        DEF_PICKLE(Portfolio);

    m.def(
      "PF_Simple",
      [](py::object tm, py::object se, py::object af, int adjust_cycle, const string& adjust_mode,
         bool delay_to_trading_day) {
          PortfolioPtr ret = make_shared<SimplePortfolio>();
          auto* ptr = (PyPortfolio*)ret.get();
          ptr->set_tm(tm);
          ptr->set_se(se);
          ptr->set_af(af);
          ret->setParam<int>("adjust_cycle", adjust_cycle);
          ret->setParam<string>("adjust_mode", adjust_mode);
          ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
          return ret;
      },
      py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
      py::arg("af") = AF_EqualWeight(), py::arg("adjust_cycle") = 1,
      py::arg("adjust_mode") = "query", py::arg("delay_to_trading_day") = true,
      R"(PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a portfolio of multiple targets with a single system strategy

    Description of the position adjustment mode adjust_mode:
    - The "query" mode follows the ktype in the input parameter query; in this case adjust_cycle is the period interval determined by the ktype in the query;
      
    - The "day" mode: adjust_cycle is the position adjustment interval in days;
    - For the "week" | "month" | "quarter" | "year" modes, adjust_cycle
      is the corresponding Nth day of the week, the Nth day of the month, the Nth day of the quarter, or the Nth day of the year; when delay_to_trading_day is false,
      if the day is not a trading day the position adjustment will be skipped; when delay_to_trading_day is true, if the day is not a trading day,
      it will be postponed to the first trading day in the current period; e.g. if the position adjustment is specified on the 1st day of each month, but the 1st day of the month is not a trading day, it will be postponed to the first trading day of that month.    
      

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param AllocateFundsBase af: the asset allocation algorithm
    :param int adjust_cycle: the position adjustment period
    :param str adjust_mode: the position adjustment mode
    :param bool delay_to_trading_day: if the day is not a trading day, it will be postponed to the first trading day in the current period)");

    m.def(
      "PF_WithoutAF",
      [](py::object tm, py::object se, int adjust_cycle, const string& adjust_mode,
         bool delay_to_trading_day, bool trade_on_close, bool sys_use_self_tm,
         bool sell_at_not_selected) {
          PortfolioPtr ret = make_shared<WithoutAFPortfolio>();
          auto* ptr = (PyPortfolio*)ret.get();
          ptr->set_tm(tm);
          ptr->set_se(se);
          ret->setParam<int>("adjust_cycle", adjust_cycle);
          ret->setParam<string>("adjust_mode", adjust_mode);
          ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
          ret->setParam<bool>("trade_on_close", trade_on_close);
          ret->setParam<bool>("sys_use_self_tm", sys_use_self_tm);
          ret->setParam<bool>("sell_at_not_selected", sell_at_not_selected);
          return ret;
      },
      py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(), py::arg("adjust_cycle") = 1,
      py::arg("adjust_mode") = "query", py::arg("delay_to_trading_day") = true,
      py::arg("trade_on_close") = true, py::arg("sys_use_self_tm") = false,
      py::arg("sell_at_not_selected") = false,
      R"(PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False,sell_at_not_selected=False])
    
    Create a portfolio without an asset allocation algorithm; all the single-system strategies use the common tm to manage the account

    Description of the position adjustment mode adjust_mode:
    - The "query" mode follows the ktype in the input parameter query; in this case adjust_cycle is the period interval determined by the ktype in the query;
      
    - The "day" mode: adjust_cycle is the position adjustment interval in days;
    - For the "week" | "month" | "quarter" | "year" modes, adjust_cycle
      is the corresponding Nth day of the week, the Nth day of the month, the Nth day of the quarter, or the Nth day of the year; when delay_to_trading_day is false,
      if the day is not a trading day the position adjustment will be skipped; when delay_to_trading_day is true, if the day is not a trading day,
      it will be postponed to the first trading day in the current period; e.g. if the position adjustment is specified on the 1st day of each month, but the 1st day of the month is not a trading day, it will be postponed to the first trading day of that month.    
      

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param int adjust_cycle: the position adjustment period
    :param str adjust_mode: the position adjustment mode
    :param bool delay_to_trading_day: if the day is not a trading day, it will be postponed to the first trading day in the current period
    :param bool trade_on_close: whether the trade is executed at the close
    :param bool sys_use_self_tm: the prototype systems use their own tm to calculate
    :param bool sell_at_not_selected: whether the stocks not selected on the position adjustment day are forcibly sold)");
}