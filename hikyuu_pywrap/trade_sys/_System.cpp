/*
 * _System.cpp
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/system/build_in.h>
#include <hikyuu/trade_sys/system/MomentResult.h>
#include <hikyuu/trade_sys/system/TradeSuggestion.h>
#include <hikyuu/trade_sys/system/SubSystemContext.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
// v5: the PF compatibility layer (factory pass-through, see docs/design/pf_af_compat/design.md §4)
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"
#include "_System.h"

namespace py = pybind11;
using namespace hku;

namespace {

// DatetimeList (std::vector<Datetime>) has been registered as an independent Python type by
// py::bind_vector (hikyuu_pywrap/bind_stl.cpp), the registered type takes precedence over the
// pybind11/stl.h converter, so it only accepts DatetimeList instances and rejects the Python
// list/tuple. Here the binding layer accepts any iterable Datetime sequence uniformly and converts
// it manually, which is compatible with the existing DatetimeList argument and also supports the
// list/tuple syntax.
DatetimeList toDatetimeList(const py::object& dates) {
    DatetimeList result;
    if (dates.is_none()) {
        return result;
    }
    if (!py::hasattr(dates, "__iter__")) {
        throw py::type_error(
          "dates must be an iterable sequence of Datetime (list/tuple/DatetimeList)");
    }
    for (auto item : py::iter(dates)) {
        result.push_back(py::cast<Datetime>(item));
    }
    return result;
}

}  // namespace

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

PySystem::PySystem(const System& base) : System(base) {}

PySystem::~PySystem() {}

void PySystem::run(const KData& kdata, bool reset, bool resetAll) {
    PYBIND11_OVERLOAD(void, System, run, kdata, reset, resetAll);
}

MomentResult PySystem::runMoment(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, System, runMoment, datetime);
}

MomentResult PySystem::runMomentOnOpen(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, System, runMomentOnOpen, datetime);
}

MomentResult PySystem::runMomentOnClose(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, System, runMomentOnClose, datetime);
}

void PySystem::readyForRun() {
    PYBIND11_OVERLOAD(void, System, readyForRun);
}

void PySystem::_reset() {
    PYBIND11_OVERLOAD(void, System, _reset);
}

void PySystem::_forceResetAll() {
    PYBIND11_OVERLOAD(void, System, _forceResetAll);
}

string PySystem::str() const {
    PYBIND11_OVERLOAD(string, System, str);
}

void PySystem::set_mm(py::object mm) {
    py::gil_scoped_acquire gil;
    auto tmp_mm = mm;
    setMM(mm.cast<MMPtr>());
    tmp_mm.release();
}

void PySystem::set_ev(py::object ev) {
    py::gil_scoped_acquire gil;
    auto tmp_ev = ev;
    setEV(ev.cast<EnvironmentPtr>());
    tmp_ev.release();
}

void PySystem::set_cn(py::object cn) {
    py::gil_scoped_acquire gil;
    auto tmp_cn = cn;
    setCN(cn.cast<CNPtr>());
    tmp_cn.release();
}

void PySystem::set_sg(py::object sg) {
    py::gil_scoped_acquire gil;
    auto tmp_sg = sg;
    setSG(sg.cast<SGPtr>());
    tmp_sg.release();
}

void PySystem::set_st(py::object st) {
    py::gil_scoped_acquire gil;
    auto tmp_st = st;
    setST(st.cast<StoplossPtr>());
    tmp_st.release();
}

void PySystem::set_tp(py::object tp) {
    py::gil_scoped_acquire gil;
    auto tmp_tp = tp;
    setTP(tp.cast<StoplossPtr>());
    tmp_tp.release();
}

void PySystem::set_pg(py::object pg) {
    py::gil_scoped_acquire gil;
    auto tmp_pg = pg;
    setPG(pg.cast<PGPtr>());
    tmp_pg.release();
}

void PySystem::set_sp(py::object sp) {
    py::gil_scoped_acquire gil;
    auto tmp_sp = sp;
    setSP(sp.cast<SlippagePtr>());
    tmp_sp.release();
}

void PySystem::set_tm(py::object tm) {
    py::gil_scoped_acquire gil;
    auto tmp_tm = tm;
    setTM(tm.cast<TradeManagerPtr>());
    tmp_tm.release();
}

PyMultiSystem::PyMultiSystem(const MultiSystem& base) : MultiSystem(base) {}

PyMultiSystem::~PyMultiSystem() {}

void PyMultiSystem::run(const KData& kdata, bool reset, bool resetAll) {
    PYBIND11_OVERLOAD(void, MultiSystem, run, kdata, reset, resetAll);
}

MomentResult PyMultiSystem::runMoment(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, MultiSystem, runMoment, datetime);
}

MomentResult PyMultiSystem::runMomentOnOpen(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, MultiSystem, runMomentOnOpen, datetime);
}

MomentResult PyMultiSystem::runMomentOnClose(const Datetime& datetime) {
    PYBIND11_OVERLOAD(MomentResult, MultiSystem, runMomentOnClose, datetime);
}

void PyMultiSystem::readyForRun() {
    PYBIND11_OVERLOAD(void, MultiSystem, readyForRun);
}

void PyMultiSystem::_reset() {
    PYBIND11_OVERLOAD(void, MultiSystem, _reset);
}

void PyMultiSystem::_forceResetAll() {
    PYBIND11_OVERLOAD(void, MultiSystem, _forceResetAll);
}

string PyMultiSystem::str() const {
    PYBIND11_OVERLOAD(string, MultiSystem, str);
}

void PyMultiSystem::set_mm(py::object mm) {
    py::gil_scoped_acquire gil;
    auto tmp_mm = mm;
    setMM(mm.cast<MMPtr>());
    tmp_mm.release();
}

void PyMultiSystem::set_ev(py::object ev) {
    py::gil_scoped_acquire gil;
    auto tmp_ev = ev;
    setEV(ev.cast<EnvironmentPtr>());
    tmp_ev.release();
}

void PyMultiSystem::set_cn(py::object cn) {
    py::gil_scoped_acquire gil;
    auto tmp_cn = cn;
    setCN(cn.cast<CNPtr>());
    tmp_cn.release();
}

void PyMultiSystem::set_sg(py::object sg) {
    py::gil_scoped_acquire gil;
    auto tmp_sg = sg;
    setSG(sg.cast<SGPtr>());
    tmp_sg.release();
}

void PyMultiSystem::set_st(py::object st) {
    py::gil_scoped_acquire gil;
    auto tmp_st = st;
    setST(st.cast<StoplossPtr>());
    tmp_st.release();
}

void PyMultiSystem::set_tp(py::object tp) {
    py::gil_scoped_acquire gil;
    auto tmp_tp = tp;
    setTP(tp.cast<StoplossPtr>());
    tmp_tp.release();
}

void PyMultiSystem::set_pg(py::object pg) {
    py::gil_scoped_acquire gil;
    auto tmp_pg = pg;
    setPG(pg.cast<PGPtr>());
    tmp_pg.release();
}

void PyMultiSystem::set_sp(py::object sp) {
    py::gil_scoped_acquire gil;
    auto tmp_sp = sp;
    setSP(sp.cast<SlippagePtr>());
    tmp_sp.release();
}

void PyMultiSystem::set_tm(py::object tm) {
    py::gil_scoped_acquire gil;
    auto tmp_tm = tm;
    setTM(tm.cast<TradeManagerPtr>());
    tmp_tm.release();
}

void export_System(py::module& m) {
    m.def("get_system_part_name", getSystemPartName, R"(get_system_part_name(part)

    Get the string name of the part
    
        - System.Part.ENVIRONMENT  - "EV"
        - System.Part.CONDITION    - "CN"
        - System.Part.SIGNAL       - "SG"
        - System.Part.STOPLOSS     - "ST"
        - System.Part.TAKEPROFIT   - "TP"
        - System.Part.MONEYMANAGER - "MM"
        - System.Part.PROFITGOAL   - "PG"
        - System.Part.SLIPPAGE     - "SP"
        - System.Part.INVALID      - "--"

    :param int part: the System.Part enumeration value
    :rtype: str)");

    m.def("get_system_part_enum", getSystemPartEnum, R"(get_system_part_enum(part_name)

     Get the corresponding enumeration value by the string name of the system part

    :param str part_name: the string name of the system part, see: :py:func:`getSystemPartName`
    :rtype: System.Part)");

    //--------------------------------------------------------------------------------------
    // Recursive combination refactoring: the extensible information model (MomentResult /
    // TradeSuggestion / SubSystemContext)
    py::enum_<SuggestionType>(m, "SuggestionType", "The suggestion type")
      .value("HOLD", SuggestionType::HOLD)
      .value("BUY", SuggestionType::BUY)
      .value("SELL", SuggestionType::SELL)
      .value("CLEAR", SuggestionType::CLEAR);

    py::class_<TradeSuggestion>(
      m, "TradeSuggestion",
      "The suggestion instruction with the complete semantic expression (without normalization)")
      .def(py::init<>())
      .def_readwrite("stock", &TradeSuggestion::stock)
      .def_readwrite("sys", &TradeSuggestion::sys)
      .def_readwrite("type", &TradeSuggestion::type)
      .def_readwrite("number", &TradeSuggestion::number)
      .def_readwrite("plan_price", &TradeSuggestion::plan_price)
      .def_readwrite("plan_cash", &TradeSuggestion::plan_cash)
      .def_readwrite("cash_ratio", &TradeSuggestion::cash_ratio)
      .def_readwrite("assets_ratio", &TradeSuggestion::assets_ratio)
      .def_readwrite("target_position_ratio", &TradeSuggestion::target_position_ratio)
      .def_readwrite("stoploss", &TradeSuggestion::stoploss)
      .def_readwrite("goalPrice", &TradeSuggestion::goalPrice)
      .def_readwrite("from", &TradeSuggestion::from)
      .def_readwrite("urgency", &TradeSuggestion::urgency)
      .def_readwrite("score", &TradeSuggestion::score)
      .def_readwrite("remark", &TradeSuggestion::remark);

    py::class_<MomentResult>(
      m, "MomentResult",
      "The complete running result (suggestion) of the system instance at a certain moment")
      .def(py::init<>())
      .def_readwrite("datetime", &MomentResult::datetime)
      .def_readwrite("funds_before_open", &MomentResult::funds_before_open)
      .def_readwrite("funds_before_close", &MomentResult::funds_before_close)
      .def_readwrite("funds", &MomentResult::funds)
      .def_readwrite("positions", &MomentResult::positions)
      .def_readwrite("tradesOnOpen", &MomentResult::tradesOnOpen)
      .def_readwrite("tradesOnClose", &MomentResult::tradesOnClose)
      .def_readwrite("delayOnNextOpen", &MomentResult::delayOnNextOpen)
      .def_readwrite("suggestions", &MomentResult::suggestions)
      .def("allTrades", &MomentResult::allTrades)
      .def("empty", &MomentResult::empty);

    py::class_<SubSystemContext>(m, "SubSystemContext",
                                 "The MM L1 context (including the mode B quota)")
      .def(py::init<>())
      .def_readwrite("sys", &SubSystemContext::sys)
      .def_readwrite("funds", &SubSystemContext::funds)
      .def_readwrite("profit_curve", &SubSystemContext::profit_curve)
      .def_readwrite("total_return", &SubSystemContext::total_return)
      .def_readwrite("current_weight", &SubSystemContext::current_weight)
      .def_readwrite("score", &SubSystemContext::score)
      .def_readwrite("quota", &SubSystemContext::quota)
      .def_readwrite("suggestion_count", &SubSystemContext::suggestion_count);

    //--------------------------------------------------------------------------------------
    py::class_<TradeRequest>(
      m, "TradeRequest",
      R"(The trade request record. The trade request information registered inside the system when implementing the delayed operation. The main purpose of exposing this structure is to
in the "delay" mode (delaying the trade to the open of the next bar), the system actually knows that the next Bar will
trade; at this time, you can know through System.getBuyTradeRequestList() and System.getSellTradeRequestList()
whether the next BAR needs to buy/sell. It is mainly used to remind or print the operations needed for the next Bar. For the system
itself, it has no effect on the operation.)")

      .def(py::init<>())
      .def("__str__", to_py_str<TradeRequest>)
      .def("__repr__", to_py_str<TradeRequest>)

      .def_readwrite("valid", &TradeRequest::valid,
                     "Whether this trade request record is valid (True | False)")
      .def_readwrite("business", &TradeRequest::business,
                     "The trade business type, see: :py:class:`hikyuu.trade_manage.BUSINESS`")
      .def_readwrite("datetime", &TradeRequest::datetime,
                     "The moment when the trade request was issued")
      .def_readwrite("stoploss", &TradeRequest::stoploss,
                     "The stop-loss price at the moment when the trade request was issued")
      .def_readwrite("part", &TradeRequest::from,
                     "The source of the trade request, see: :py:class:`System.Part`")
      .def_readwrite("count", &TradeRequest::count,
                     "The number of the consecutive delays due to the operation failures")
        DEF_PICKLE(TradeRequest);

    //--------------------------------------------------------------------------------------
    py::class_<System, SystemPtr, PySystem>(
      m, "System", py::dynamic_attr(),
      R"(The system base class. To extend or implement the more complex system trading behaviors, you can inherit from this class.

A system refers to the complete strategy for a single trading object, including the market environment, the system valid condition, the money management, the stop-loss, the take-profit, the profit goal and the slippage; it is used for the simulated backtesting.

Common parameters:

  - delay=True (bool): whether to delay the trade to the open of the next bar
  - delay_use_current_price=True (bool): in the case of the delayed operation, whether to calculate the new stop-loss/take-profit/target price with the price of the bar at the current trade, or use the result calculated last time
  - max_delay_count=3 (int): the limit on the number of the consecutive delayed trade requests; it should be greater than or equal to 0, and 0 means only one delay is allowed
  - tp_monotonic=True (bool): the take-profit increases monotonically
  - tp_delay_n=3 (int): the number of the days when the take-profit delay starts, i.e. the take-profit strategy judgment takes effect only after several days of the actual trading
  - ignore_sell_sg=False (bool): ignore the sell signal, and sell only by the stop-loss/take-profit and the other ways
  - ev_open_position=False (bool): whether to use the market environment for the initial position building
  - cn_open_position=False (bool): whether to use the system valid condition for the initial position building)")

      .def(py::init<const string&>())
      .def(py::init<const System&>())
      .def(py::init<const TradeManagerPtr&, const MoneyManagerPtr&, const EnvironmentPtr&,
                    const ConditionPtr&, const SignalPtr&, const StoplossPtr&, const StoplossPtr&,
                    const ProfitGoalPtr&, const SlippagePtr&, const string&>())
      .def("__str__", to_py_str<System>)
      .def("__repr__", to_py_str<System>)

      .def_property("name", py::overload_cast<>(&System::name, py::const_),
                    py::overload_cast<const string&>(&System::name), py::return_value_policy::copy,
                    "The system name")
      .def_property_readonly("query", &System::getQuery, py::return_value_policy::copy,
                             "The query condition")

      .def_property("to", &System::getTO, &System::setTO, "The trading object KData")

      //   .def_property("tm", &System::getTM, &System::setTM, "The associated trade manager
      //   instance")

      .def_property(
        "tm", &System::getTM, [](PySystem& self, py::object py_tm) { self.set_tm(py_tm); },
        "The associated trade manager instance")
      .def_property(
        "mm", &System::getMM, [](PySystem& self, py::object py_mm) { self.set_mm(py_mm); },
        "The money management strategy")
      .def_property(
        "ev", &System::getEV, [](PySystem& self, py::object py_ev) { self.set_ev(py_ev); },
        "The market environment strategy")
      .def_property(
        "cn", &System::getCN, [](PySystem& self, py::object py_tm) { self.set_cn(py_tm); },
        "The system valid condition")
      .def_property(
        "sg", &System::getSG, [](PySystem& self, py::object py_sig) { self.set_sg(py_sig); },
        "The signal generator")
      .def_property(
        "st", &System::getST, [](PySystem& self, py::object py_st) { self.set_st(py_st); },
        "The stop-loss strategy")
      .def_property(
        "tp", &System::getTP, [](PySystem& self, py::object py_tp) { self.set_tp(py_tp); },
        "The take-profit strategy")
      .def_property(
        "pg", &System::getPG, [](PySystem& self, py::object py_pg) { self.set_pg(py_pg); },
        "The profit goal strategy")
      .def_property(
        "sp", &System::getSP, [](PySystem& self, py::object py_sp) { self.set_sp(py_sp); },
        "The slippage algorithm")

      .def("get_param", &System::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (System::*)(const std::string&, const boost::any&)>(&System::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &System::haveParam, "Whether the specified parameter exists")

      .def("set_not_shared_all", &System::setNotSharedAll, "Set all the parts to non-shared")

      .def("get_stock", &System::getStock, R"(get_stock(self)

    Get the associated security

    :rtype: Stock)")

      .def("get_trade_record_list", &System::getTradeRecordList, py::return_value_policy::copy,
           R"(get_trade_record_list(self)

    Get the actually executed trade records; the difference from the TM is that it does not contain the trade records caused by the dividend adjustments

    :rtype: TradeRecordList)")

      .def("get_buy_trade_request_list", &System::getBuyTradeRequestList,
           py::return_value_policy::copy,
           R"(get_buy_trade_request_list(self)
  
    Get the buy request list; in the "delay" mode, check whether there is a buy operation at the next moment

    :rtype: list[TradeRequest])")

      .def("get_sell_trade_request_list", &System::getSellTradeRequestList,
           py::return_value_policy::copy,
           R"(get_sell_trade_request_list(self)

    Get the sell request list; in the "delay" mode, check whether there is a sell operation at the next moment

    :rtype: list[TradeRequest])")

      .def("get_sell_short_trade_request_list", &System::getSellShortTradeRequestList,
           py::return_value_policy::copy)
      .def("get_buy_short_trade_request_list", &System::getBuyShortTradeRequestList,
           py::return_value_policy::copy)

      .def("reset", &System::reset,
           R"(reset(self)

    Reset, but excluding the existing trading object and the shared parts.)")

      .def("force_reset_all", &System::forceResetAll,
           R"(force_reset_all(self)

    Forcibly reset all the parts and clear the existing trading object, ignoring the sharing attributes of the parts.)")

      .def("clone", &System::clone,
           R"(clone(self)

    The clone operation; it clones according to the sharing attributes of the parts; the shared parts are not actually cloned and remain shared.)")

      .def("run", py::overload_cast<const KQuery&, bool, bool>(&System::run), py::arg("query"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const KData&, bool, bool>(&System::run), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const Stock&, const KQuery&, bool, bool>(&System::run),
           py::arg("stock"), py::arg("query"), py::arg("reset") = true,
           py::arg("reset_all") = false,
           R"(run(self, stock, query[, reset=True])
  
    Run the system, performing the backtest

    :param Stock stock: the security to trade
    :param Query query: the K-line data query condition
    :param bool reset: whether to reset according to the sharing attributes of the system parts before executing
    :param bool reset_all: forcibly reset all the parts)")

      .def("ready", &System::readyForRun)

      .def(
        "last_suggestion",
        [](const System& system) {
            json j = system.lastSuggestion();
            std::string json_str = j.dump();
            py::module json_module = py::module::import("json");
            return json_module.attr("loads")(json_str);
        },
        "After the backtest is completed, return the trade records of the last day, and the "
        "delayed buy and sell requests that need to be delayed")

        DEF_PICKLE(System);

    //--------------------------------------------------------------------------------------
    // Recursive combination refactoring: the aggregate trading system (portfolio backtesting)
    py::class_<MultiSystem, System, std::shared_ptr<MultiSystem>>(
      m, "MultiSystem", py::dynamic_attr(),
      R"(The aggregate trading system (portfolio backtesting). It holds multiple sub-systems (single-security or nested aggregate), drives and aggregates the orders at the open/close stages respectively.
Every sub-system has its own independent virtual account (a shadow account in mode A / the quota allocated by the parent in mode B), the parent system allocates and orders uniformly on its own account.)")
      .def(py::init<>())
      .def(py::init<const string&>(), py::arg("name") = "MultiSystem")
      .def(py::init<const SystemList&, const string&>(), py::arg("sys_list"),
           py::arg("name") = "MultiSystem")
      .def("add", &MultiSystem::add, py::arg("sys"),
           "Add a sub-system (with the circular reference detection)")
      .def("get_system_list", &MultiSystem::getSystemList, "Get the sub-system list")
      .def("run", py::overload_cast<const KData&, bool, bool>(&MultiSystem::run), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false,
           R"(run(self, kdata, reset=True, reset_all=False)

    The portfolio backtesting entry. kdata is used as the aligned time axis, it traverses every trading day and drives all the sub-systems (open/close) respectively and aggregates the orders.
    Every sub-system has an independent virtual account, the parent system allocates and orders uniformly on its own account.

    :param KData kdata: the aligned time axis (it should cover the trading days of every sub-system))")
      .def("run", py::overload_cast<const KQuery&, bool, bool>(&MultiSystem::run), py::arg("query"),
           py::arg("reset") = true, py::arg("reset_all") = false,
           R"(run(self, query, reset=True, reset_all=False)

    [master compatibility] Run with the market trading calendar as the driving axis, equivalent to the master Portfolio.run(query).

    The difference from run(kdata): the driving axis no longer takes the dates of the input KData, but takes the trading calendar
    (the injected fixed time axis takes precedence, otherwise StockManager.get_trading_calendar(query)).
    The price and ktype context is taken from the KData of the reference instrument (its own instrument -> the first sub-system instrument -> the calendar benchmark index).

    :param Query query: the query condition
    :param bool reset: whether to reset before running
    :param bool reset_all: whether to force a full reset before running)")
      .def("runMoment", &MultiSystem::runMoment, py::arg("datetime"),
           R"(runMoment(self, datetime)

    Execute one step at the specified moment, drive all the sub-systems (open/close) respectively, and aggregate the trades and suggestions.

    :param Datetime datetime: the specified date
    :rtype: MomentResult)")
      .def("runMomentOnOpen", &MultiSystem::runMomentOnOpen, py::arg("datetime"))
      .def("runMomentOnClose", &MultiSystem::runMomentOnClose, py::arg("datetime"))
      .def("ready_for_run", &MultiSystem::readyForRun)
      .def("set_mode", &MultiSystem::setMode, py::arg("mode"),
           "Set the running mode: A (signal aggregation) / B (fund allocation)")
      .def_property_readonly("mode", &MultiSystem::getMode, "The current running mode (A/B)")
      .def("set_sub_init_cash", &MultiSystem::setSubInitCash, py::arg("cash"),
           "Set the initial fund of the sub-system shadow account (a fixed value in mode A / the "
           "initial quota in mode B)")
      .def("set_adjust_cycle", &MultiSystem::setAdjustCycle, py::arg("days"),
           "Set the rebalancing cycle (days), <=1 means rebalancing on every close day")
      .def("set_axis_mode", &MultiSystem::setAxisMode, py::arg("mode"),
           R"(set_axis_mode(self, mode)

    Set the driving time axis mode: "kdata" (the default, the dates of the input KData of run(kdata) are used as the driving axis)
    or "calendar" (the fixed date table injected by set_date_axis is used as the driving axis). An invalid value is warned and it falls back to "kdata".

    :param str mode: "kdata" / "calendar")")
      .def("get_axis_mode", &MultiSystem::getAxisMode, "Get the driving time axis mode")
      .def(
        "set_date_axis",
        [](MultiSystem& ms, const py::object& dates) { ms.setDateAxis(toDatetimeList(dates)); },
        py::arg("dates"),
        "Set the fixed date table (accepting list/tuple/DatetimeList; it is used as the driving "
        "axis only when axis_mode == \"calendar\";"
        "an empty table falls back to the kdata axis with a warning)")
      .def("get_date_axis", &MultiSystem::getDateAxis, "Get the fixed date table")
      .def("clear_date_axis", &MultiSystem::clearDateAxis, "Clear the fixed date table")
      .def(
        "set_adjust_dates",
        [](MultiSystem& ms, const py::object& dates) { ms.setAdjustDates(toDatetimeList(dates)); },
        py::arg("dates"),
        R"(set_adjust_dates(self, dates)

    Set the external rebalancing day table (when it is not empty it takes precedence as the rebalancing day criterion, otherwise it falls back to the counting judgment of set_adjust_cycle).
    The input dates are normalized to the zero hour of that day and stored, the rebalancing is only executed on the dates hitting the table.

    :param dates: the Datetime sequence (list/tuple/DatetimeList are all accepted))")
      .def(
        "get_adjust_dates",
        [](const MultiSystem& ms) {
            // Return a list (DatetimeList) instead of the C++ std::set: consistent with
            // get_date_axis, and avoid the set conversion failure when Datetime has no __hash__ on
            // the Python side
            const auto& dates = ms.getAdjustDates();
            return DatetimeList(dates.begin(), dates.end());
        },
        "Get the external rebalancing day table (already normalized to the zero hour of that day)")
      .def("clear_adjust_dates", &MultiSystem::clearAdjustDates,
           "Clear the external rebalancing day table (fall back to the rebalancing cycle counting "
           "judgment)")
      .def_static(
        "calc_adjust_dates",
        [](const py::object& dates, const string& mode, int adjust_cycle,
           bool delay_to_trading_day) {
            return MultiSystem::calcAdjustDates(toDatetimeList(dates), mode, adjust_cycle,
                                                delay_to_trading_day);
        },
        py::arg("dates"), py::arg("mode"), py::arg("adjust_cycle") = 1,
        py::arg("delay_to_trading_day") = true,
        R"(calc_adjust_dates(dates, mode, adjust_cycle=1, delay_to_trading_day=True)

    [Static] Calculate the rebalancing day set on the given trading day axis (a pure function, usable to preview the rebalancing rhythm).

    :param dates: the sorted trading day sequence (list/tuple/DatetimeList)
    :param str mode: "week" / "month" / "quarter" / "year" (the other values return empty)
    :param int adjust_cycle: the N-th day within the cycle (<=0 is treated as 1)
    :param bool delay_to_trading_day: whether to postpone to the first trading day within the current cycle when the target day is not a trading day
    :rtype: DatetimeList)")
      .def("set_trade_on_close", &MultiSystem::setTradeOnClose, py::arg("on_close"),
           "Set whether to execute the rebalancing orders at the close stage")
      .def("set_adjust_mode", &MultiSystem::setAdjustMode, py::arg("mode"),
           R"(set_adjust_mode(self, mode)

    Set the rebalancing mode (taking over the master PF adjust_mode):
      - "query" / "day" (the default): continue the "every N close days" counting judgment of set_adjust_cycle;
      - "week" / "month" / "quarter" / "year": expand the rebalancing day table by "the adjust_cycle-th day within the cycle" on the driving axis.
    An invalid value is warned and it falls back to "query".

    :param str mode: "query" / "day" / "week" / "month" / "quarter" / "year")")
      .def("get_adjust_mode", &MultiSystem::getAdjustMode, "Get the rebalancing mode")
      .def("set_delay_to_trading_day", &MultiSystem::setDelayToTradingDay, py::arg("delay"),
           "Set whether to postpone to the first trading day within the current cycle when the "
           "rebalancing day is not a trading day (it takes effect only when "
           "week/month/quarter/year are expanded)")
      .def("get_delay_to_trading_day", &MultiSystem::getDelayToTradingDay,
           "Get whether the rebalancing day is postponed to the trading day")
      .def("set_se", &MultiSystem::setSE, py::arg("se"),
           "Set the trading object selector (optional, only the rebalancing-day stock selection "
           "filtering)")
      .def_property_readonly("se", &MultiSystem::getSE, "The trading object selector")
      .def("set_sell_at_not_selected", &MultiSystem::setSellAtNotSelected, py::arg("on"),
           "Set whether to force liquidating the unselected sub-systems (SE is required)")
      .def("get_adjust_turnover", &MultiSystem::getAdjustTurnover,
           "Get the turnover rate of every rebalancing day (a list of (date, turnover amount / "
           "total assets))")
      // Consistent with the set_* of PySystem: hold the GIL and release() to keep it alive when
      // setting the Python custom parts, to prevent the Python-side parts (e.g. the custom MM/SG)
      // from being GC'd early causing the C++ side to hold a dangling pointer (use-after-free).
      .def_property(
        "tm", &MultiSystem::getTM,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setTM(o.cast<TradeManagerPtr>());
            tmp.release();
        },
        "The associated trade management instance")
      .def_property(
        "mm", &MultiSystem::getMM,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setMM(o.cast<MMPtr>());
            tmp.release();
        },
        "The money management strategy")
      .def_property(
        "ev", &MultiSystem::getEV,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setEV(o.cast<EnvironmentPtr>());
            tmp.release();
        },
        "The market environment judgment strategy")
      .def_property(
        "cn", &MultiSystem::getCN,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setCN(o.cast<CNPtr>());
            tmp.release();
        },
        "The system precondition")
      .def_property(
        "sg", &MultiSystem::getSG,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setSG(o.cast<SGPtr>());
            tmp.release();
        },
        "The signal generator")
      .def_property(
        "st", &MultiSystem::getST,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setST(o.cast<StoplossPtr>());
            tmp.release();
        },
        "The stop-loss strategy")
      .def_property(
        "tp", &MultiSystem::getTP,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setTP(o.cast<StoplossPtr>());
            tmp.release();
        },
        "The take-profit strategy")
      .def_property(
        "pg", &MultiSystem::getPG,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setPG(o.cast<PGPtr>());
            tmp.release();
        },
        "The profit goal strategy")
      .def_property(
        "sp", &MultiSystem::getSP,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setSP(o.cast<SlippagePtr>());
            tmp.release();
        },
        "The slippage algorithm")
      .def_property(
        "af", &MultiSystem::getAF,
        [](MultiSystem& self, py::object o) {
            py::gil_scoped_acquire gil;
            auto tmp = o;
            self.setAF(o.cast<AllocateFundsPtr>());
            tmp.release();
        },
        "The portfolio-level fund allocation algorithm (AF, carrying L1/L2/L3; used by the "
        "aggregate system only)")
      .def("clone", &MultiSystem::clone);

    //--------------------------------------------------------------------------------------
    // v5: the PF compatibility layer (factory pass-through to MultiSystem), keeping the master call
    // style unchanged (see docs/design/pf_af_compat/design.md §4; the return type changes from
    // PortfolioPtr to MultiSystem)
    m.def("PF_Simple", &PF_Simple, py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
          py::arg("af") = AF_EqualWeight(), py::arg("adjust_cycle") = 1,
          py::arg("adjust_mode") = "query", py::arg("delay_to_trading_day") = true,
          py::keep_alive<0, 1>(), py::keep_alive<0, 2>(), py::keep_alive<0, 3>(),
          R"(PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a multi-instrument, single-system-strategy portfolio (v5: returns MultiSystem, the semantics is the mode B quota allocation)

    The rebalancing mode adjust_mode description:
    - In the "query" mode, it follows the ktype in the input parameter query, at this time adjust_cycle determines the cycle interval
      by the ktype in query;
    - In the "day" mode, adjust_cycle is the rebalancing interval days
    - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle
      is the corresponding N-th day of every week, the n-th day of every month, the n-th day of every quarter and the n-th day of
      every year; when delay_to_trading_day is false and that day is not a trading day, the rebalancing is skipped; when
      delay_to_trading_day is true and that day is not a trading day, it is postponed to the first trading day within the current
      cycle, e.g. if the rebalancing is specified on the 1st day of every month but the 1st of that month is not a trading day,
      it is postponed to the first trading day of that month.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the trading object selection algorithm
    :param AllocateFundsBase af: the portfolio-level fund allocation algorithm (AF, carrying L1/L2/L3)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode
    :param bool delay_to_trading_day: when that day is not a trading day, it is postponed to the first trading day within the current cycle
    :rtype: MultiSystem)");

    m.def(
      "PF_WithoutAF", &PF_WithoutAF, py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
      py::arg("adjust_cycle") = 1, py::arg("adjust_mode") = "query",
      py::arg("delay_to_trading_day") = true, py::arg("trade_on_close") = true,
      py::arg("sys_use_self_tm") = false, py::arg("sell_at_not_selected") = false,
      py::keep_alive<0, 1>(), py::keep_alive<0, 2>(),
      R"(PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    Create a portfolio without a fund allocation algorithm (v5: returns MultiSystem, the semantics is the mode A signal aggregation)

    The rebalancing mode adjust_mode description:
    - In the "query" mode, it follows the ktype in the input parameter query, at this time adjust_cycle determines the cycle interval
      by the ktype in query;
    - In the "day" mode, adjust_cycle is the rebalancing interval days
    - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle
      is the corresponding N-th day of every week, the n-th day of every month, the n-th day of every quarter and the n-th day of
      every year; when delay_to_trading_day is false and that day is not a trading day, the rebalancing is skipped; when
      delay_to_trading_day is true and that day is not a trading day, it is postponed to the first trading day within the current
      cycle, e.g. if the rebalancing is specified on the 1st day of every month but the 1st of that month is not a trading day,
      it is postponed to the first trading day of that month.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the trading object selection algorithm
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode
    :param bool delay_to_trading_day: when that day is not a trading day, it is postponed to the first trading day within the current cycle
    :param bool trade_on_close: whether the trade is executed at the close
    :param bool sys_use_self_tm: the prototype system uses its own tm for the calculation (ignored with a warning in v5)
    :param bool sell_at_not_selected: whether to force selling the stocks not selected on the rebalancing day
    :rtype: MultiSystem)");

    //--------------------------------------------------------------------------------------
    m.def(
      "SYS_Simple",
      [](py::object tm = py::none(), py::object mm = py::none(), py::object ev = py::none(),
         py::object cn = py::none(), py::object sg = py::none(), py::object st = py::none(),
         py::object tp = py::none(), py::object pg = py::none(), py::object sp = py::none()) {
          auto sys = make_shared<System>("SYS_Simple");
          auto* sys_ptr = (PySystem*)sys.get();
          sys_ptr->set_tm(tm);
          sys_ptr->set_mm(mm);
          sys_ptr->set_ev(ev);
          sys_ptr->set_cn(cn);
          sys_ptr->set_sg(sg);
          sys_ptr->set_st(st);
          sys_ptr->set_tp(tp);
          sys_ptr->set_pg(pg);
          sys_ptr->set_sp(sp);
          return sys;
      },
      py::arg("tm") = py::none(), py::arg("mm") = py::none(), py::arg("ev") = py::none(),
      py::arg("cn") = py::none(), py::arg("sg") = py::none(), py::arg("st") = py::none(),
      py::arg("tp") = py::none(), py::arg("pg") = py::none(), py::arg("sp") = py::none(),
      R"(SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

  Create a simple system instance (no multiple position increases or decreases per trade, i.e. after each buy, sell all when selling); when the system instance runs (calling the run method), it needs at least a matching trade manager instance, a money management strategy
  and a signal generator), which can be specified after creating the system instance. If there is no output when calling run,
  and no correct results, it may be that tm, sg, mm are not set. For the backtest, use the run method, e.g.::
    
        # Create a simulated trading account for the backtest, with an initial capital of 300,000
        my_tm = crtTM(init_cash = 300000)

        # Create the signal generator (with the 5-day EMA as the fast line and the 10-day EMA of the 5-day EMA itself as the slow line; buy when the fast line crosses the slow line upward, and sell otherwise)
        my_sg = SG_Flex(EMA(C, n=5), slow_n=10)

        # Fixedly buy 1000 shares each time
        my_mm = MM_FixedCount(1000)

        # Create the trading system and run it
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))
    
    :param TradeManager tm: the trade manager instance 
    :param MoneyManager mm: the money management strategy
    :param EnvironmentBase ev: the market environment strategy
    :param ConditionBase cn: the system valid condition
    :param SignalBase sg: the signal generator
    :param StoplossBase st: the stop-loss strategy
    :param StoplossBase tp: the take-profit strategy
    :param ProfitGoalBase pg: the profit goal strategy
    :param SlippageBase sp: the slippage algorithm
    :return: the system instance)");

    m.def(
      "SYS_WalkForward",
      [](const py::sequence& candidate_sys_list, const TradeManagerPtr& tm, size_t train_len,
         size_t test_len, const SelectorPtr& se, const TradeManagerPtr& train_tm) {
          SystemList sys_list = python_list_to_vector<SystemPtr>(candidate_sys_list);
          SelectorPtr c_se = se;
          if (!c_se) {
              c_se = SE_PerformanceOptimal();
          }
          return SYS_WalkForward(sys_list, tm, train_len, test_len, c_se, train_tm);
      },
      py::arg("sys_list"), py::arg("tm") = TradeManagerPtr(), py::arg("train_len") = 100,
      py::arg("test_len") = 20, py::arg("se") = SelectorPtr(),
      py::arg("train_tm") = TradeManagerPtr(),
      R"(SYS_WalkForward(sys_list, tm, train_len, test_len, train_tm)

  Create a walk-forward optimization system; when there is only one candidate system in the input candidate system list, it is a walk-forward system

  :param sequence sys_list: the candidate system list
  :param TradeManager tm: the trading account
  :param int train_len: the data length used when evaluating the system performance in the rolling way
  :param int test_len: the data length executed with the optimal system selected in train_len
  :param SelectorBase se: the optimization selector, defaults to selecting the one with the maximum "Account Avg Annual Return %"
  :param TradeManager train_tm: the trading account used in the rolling evaluation; when it is None, a copy of tm is used for the evaluation)");
}