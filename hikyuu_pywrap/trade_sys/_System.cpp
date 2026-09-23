/*
 * _System.cpp
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/system/build_in.h>
#include "../pybind_utils.h"
#include "_System.h"

namespace py = pybind11;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

PySystem::PySystem(const System& base) : System(base) {}

PySystem::~PySystem() {}

void PySystem::run(const KData& kdata, bool reset, bool resetAll) {
    PYBIND11_OVERLOAD(void, System, run, kdata, reset, resetAll);
}

TradeRecord PySystem::runMoment(const Datetime& datetime) {
    PYBIND11_OVERLOAD(TradeRecord, System, runMoment, datetime);
}

TradeRecord PySystem::runMomentOnOpen(const Datetime& datetime) {
    PYBIND11_OVERLOAD(TradeRecord, System, runMomentOnOpen, datetime);
}

TradeRecord PySystem::runMomentOnClose(const Datetime& datetime) {
    PYBIND11_OVERLOAD(TradeRecord, System, runMomentOnClose, datetime);
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
    py::class_<TradeRequest>(
      m, "TradeRequest",
      R"(The trade request record. The trade request information registered inside the system when implementing the delayed operation. The main purpose of exposing this structure is to
in the "delay" mode (delaying the trade to the open of the next bar), the system actually knows that the next Bar will
trade; at this time, you can know through System.getBuyTradeRequest() and System.getSellTradeRequest()
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

      .def("set_not_shared_all", &System::setNotSharedAll, "Set all the components to non-shared")

      .def("get_stock", &System::getStock, R"(get_stock(self)

    Get the associated security

    :rtype: Stock)")

      .def("get_trade_record_list", &System::getTradeRecordList, py::return_value_policy::copy,
           R"(get_trade_record_list(self)

    Get the actually executed trade records; the difference from the TM is that it does not contain the trade records caused by the dividend adjustments

    :rtype: TradeRecordList)")

      .def("get_buy_trade_request", &System::getBuyTradeRequest, py::return_value_policy::copy,
           R"(get_buy_trade_request(self)
  
    Get the buy request; in the "delay" mode, check whether there is a buy operation at the next moment

    :rtype: TradeRequest)")

      .def("get_sell_trade_request", &System::getSellTradeRequest, py::return_value_policy::copy,
           R"(get_sell_trade_request(self)

    Get the sell request; in the "delay" mode, check whether there is a sell operation at the next moment

    :rtype: TradeRequest)")

      .def("get_sell_short_trade_request", &System::getSellShortTradeRequest,
           py::return_value_policy::copy)
      .def("get_buy_short_trade_request", &System::getBuyShortTradeRequest,
           py::return_value_policy::copy)

      .def("reset", &System::reset,
           R"(reset(self)

    Reset, but excluding the existing trading object and the shared parts.)")

      .def("force_reset_all", &System::forceResetAll,
           R"(force_reset_all(self)

    Forcibly reset all the components and clear the existing trading object, ignoring the sharing attributes of the components.)")

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