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
// v5：PF 兼容层（工厂直通，见 docs/design/pf_af_compat/design.md §4）
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"
#include "_System.h"

namespace py = pybind11;
using namespace hku;

namespace {

// DatetimeList（std::vector<Datetime>）已由 py::bind_vector 注册为独立 Python 类型
// （hikyuu_pywrap/bind_stl.cpp），注册类型优先于 pybind11/stl.h 的转换器，导致仅接受
// DatetimeList 实例而拒绝 Python list/tuple。此处在绑定层统一接受任意可迭代的 Datetime
// 序列并手动转换，既兼容既有的 DatetimeList 传参，也支持 list/tuple 写法。
DatetimeList toDatetimeList(const py::object& dates) {
    DatetimeList result;
    if (dates.is_none()) {
        return result;
    }
    if (!py::hasattr(dates, "__iter__")) {
        throw py::type_error("dates 需为 Datetime 的可迭代序列（list/tuple/DatetimeList）");
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

    获取部件的字符串名称
    
        - System.Part.ENVIRONMENT  - "EV"
        - System.Part.CONDITION    - "CN"
        - System.Part.SIGNAL       - "SG"
        - System.Part.STOPLOSS     - "ST"
        - System.Part.TAKEPROFIT   - "TP"
        - System.Part.MONEYMANAGER - "MM"
        - System.Part.PROFITGOAL   - "PG"
        - System.Part.SLIPPAGE     - "SP"
        - System.Part.INVALID      - "--"

    :param int part: System.Part 枚举值
    :rtype: str)");

    m.def("get_system_part_enum", getSystemPartEnum, R"(get_system_part_enum(part_name)

     根据系统部件的字符串名称获取相应的枚举值

    :param str part_name: 系统部件的字符串名称，参见：:py:func:`getSystemPartName`
    :rtype: System.Part)");

    //--------------------------------------------------------------------------------------
    // 递归组合重构：可扩展信息模型（MomentResult / TradeSuggestion / SubSystemContext）
    py::enum_<SuggestionType>(m, "SuggestionType", "建议类型")
      .value("HOLD", SuggestionType::HOLD)
      .value("BUY", SuggestionType::BUY)
      .value("SELL", SuggestionType::SELL)
      .value("CLEAR", SuggestionType::CLEAR);

    py::class_<TradeSuggestion>(m, "TradeSuggestion", "完整语义表达的建议指令（不做归一化）")
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

    py::class_<MomentResult>(m, "MomentResult", "某一时刻系统实例的完整运行结果（建议）")
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

    py::class_<SubSystemContext>(m, "SubSystemContext", "MM L1 上下文（含模式 B 额度）")
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
      R"(交易请求记录。系统内部在实现延迟操作时登记的交易请求信息。暴露该结构的主要目的是用于
在“delay”模式（延迟到下一个bar开盘时进行交易）的情况下，系统实际已知下一个Bar将要
进行交易，此时可通过 System.getBuyTradeRequestList() 、 System.getSellTradeRequestList()
来获知下一个BAR是否需要买入/卖出。主要用于提醒或打印下一个Bar需要进行操作。对于系统
本身的运行没有影响。)")

      .def(py::init<>())
      .def("__str__", to_py_str<TradeRequest>)
      .def("__repr__", to_py_str<TradeRequest>)

      .def_readwrite("valid", &TradeRequest::valid, "该交易请求记录是否有效（True | False）")
      .def_readwrite("business", &TradeRequest::business,
                     "交易业务类型，参见：:py:class:`hikyuu.trade_manage.BUSINESS`")
      .def_readwrite("datetime", &TradeRequest::datetime, "发出交易请求的时刻")
      .def_readwrite("stoploss", &TradeRequest::stoploss, "发出交易请求时刻的止损价")
      .def_readwrite("part", &TradeRequest::from,
                     "发出交易请求的来源，参见：:py:class:`System.Part`")
      .def_readwrite("count", &TradeRequest::count, "因操作失败，连续延迟的次数")
        DEF_PICKLE(TradeRequest);

    //--------------------------------------------------------------------------------------
    py::class_<System, SystemPtr, PySystem>(
      m, "System", py::dynamic_attr(),
      R"(系统基类。需要扩展或实现更复杂的系统交易行为，可从此类继承。

系统是指针对单个交易对象的完整策略，包括环境判断、系统有效条件、资金管理、止损、止盈、盈利目标、移滑价差的完整策略，用于模拟回测。

公共参数：

  - delay=True (bool) : 是否延迟到下一个bar开盘时进行交易
  - delay_use_current_price=True (bool) : 延迟操作的情况下，是使用当前交易时bar的价格计算新的止损价/止赢价/目标价还是使用上次计算的结果
  - max_delay_count=3 (int) : 连续延迟交易请求的限制次数，应大于等于0，0表示只允许延迟1次
  - tp_monotonic=True (bool) : 止赢单调递增
  - tp_delay_n=3 (int) : 止盈延迟开始的天数，即止盈策略判断从实际交易几天后开始生效
  - ignore_sell_sg=False (bool) : 忽略卖出信号，只使用止损/止赢等其他方式卖出
  - ev_open_position=False (bool): 是否使用市场环境判定进行初始建仓
  - cn_open_position=False (bool): 是否使用系统有效性条件进行初始建仓)")

      .def(py::init<const string&>())
      .def(py::init<const System&>())
      .def(py::init<const TradeManagerPtr&, const MoneyManagerPtr&, const EnvironmentPtr&,
                    const ConditionPtr&, const SignalPtr&, const StoplossPtr&, const StoplossPtr&,
                    const ProfitGoalPtr&, const SlippagePtr&, const string&>())
      .def("__str__", to_py_str<System>)
      .def("__repr__", to_py_str<System>)

      .def_property("name", py::overload_cast<>(&System::name, py::const_),
                    py::overload_cast<const string&>(&System::name), py::return_value_policy::copy,
                    "系统名称")
      .def_property_readonly("query", &System::getQuery, py::return_value_policy::copy, "查询条件")

      .def_property("to", &System::getTO, &System::setTO, "交易对象 KData")

      //   .def_property("tm", &System::getTM, &System::setTM, "关联的交易管理实例")

      .def_property(
        "tm", &System::getTM, [](PySystem& self, py::object py_tm) { self.set_tm(py_tm); },
        "关联的交易管理实例")
      .def_property(
        "mm", &System::getMM, [](PySystem& self, py::object py_mm) { self.set_mm(py_mm); },
        "资金管理策略")
      .def_property(
        "ev", &System::getEV, [](PySystem& self, py::object py_ev) { self.set_ev(py_ev); },
        "市场环境判断策略")
      .def_property(
        "cn", &System::getCN, [](PySystem& self, py::object py_tm) { self.set_cn(py_tm); },
        "系统有效条件")
      .def_property(
        "sg", &System::getSG, [](PySystem& self, py::object py_sig) { self.set_sg(py_sig); },
        "信号指示器")
      .def_property(
        "st", &System::getST, [](PySystem& self, py::object py_st) { self.set_st(py_st); },
        "止损策略")
      .def_property(
        "tp", &System::getTP, [](PySystem& self, py::object py_tp) { self.set_tp(py_tp); },
        "止盈策略")
      .def_property(
        "pg", &System::getPG, [](PySystem& self, py::object py_pg) { self.set_pg(py_pg); },
        "盈利目标策略")
      .def_property(
        "sp", &System::getSP, [](PySystem& self, py::object py_sp) { self.set_sp(py_sp); },
        "移滑价差算法")

      .def("get_param", &System::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param",
           static_cast<void (System::*)(const std::string&, const boost::any&)>(&System::setParam),
           R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &System::haveParam, "是否存在指定参数")

      .def("set_not_shared_all", &System::setNotSharedAll, "将所有组件设置为非共享")

      .def("get_stock", &System::getStock, R"(get_stock(self)

    获取关联的证券

    :rtype: Stock)")

      .def("get_trade_record_list", &System::getTradeRecordList, py::return_value_policy::copy,
           R"(get_trade_record_list(self)

    获取实际执行的交易记录，和 TM 的区别是不包含权息调整带来的交易记录

    :rtype: TradeRecordList)")

      .def("get_buy_trade_request_list", &System::getBuyTradeRequestList,
           py::return_value_policy::copy,
           R"(get_buy_trade_request_list(self)
  
    获取买入请求列表，“delay”模式下查看下一时刻是否存在买入操作

    :rtype: list[TradeRequest])")

      .def("get_sell_trade_request_list", &System::getSellTradeRequestList,
           py::return_value_policy::copy,
           R"(get_sell_trade_request_list(self)

    获取卖出请求列表，“delay”模式下查看下一时刻是否存在卖出操作

    :rtype: list[TradeRequest])")

      .def("get_sell_short_trade_request_list", &System::getSellShortTradeRequestList,
           py::return_value_policy::copy)
      .def("get_buy_short_trade_request_list", &System::getBuyShortTradeRequestList,
           py::return_value_policy::copy)

      .def("reset", &System::reset,
           R"(reset(self)

    复位，但不包括已有的交易对象，以及共享的部件。)")

      .def("force_reset_all", &System::forceResetAll,
           R"(force_reset_all(self)

    强制复位所有组件以及清空已有的交易对象，忽略组件的共享属性。)")

      .def("clone", &System::clone,
           R"(clone(self)

    克隆操作，会依据部件的共享特性进行克隆，共享部件不进行实际的克隆操作，保持共享。)")

      .def("run", py::overload_cast<const KQuery&, bool, bool>(&System::run), py::arg("query"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const KData&, bool, bool>(&System::run), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const Stock&, const KQuery&, bool, bool>(&System::run),
           py::arg("stock"), py::arg("query"), py::arg("reset") = true,
           py::arg("reset_all") = false,
           R"(run(self, stock, query[, reset=True])
  
    运行系统，执行回测

    :param Stock stock: 交易的证券
    :param Query query: K线数据查询条件
    :param bool reset: 执行前是否依据系统部件共享属性复位
    :param bool reset_all: 强制复位所有部件)")

      .def("ready", &System::readyForRun)

      .def(
        "last_suggestion",
        [](const System& system) {
            json j = system.lastSuggestion();
            std::string json_str = j.dump();
            py::module json_module = py::module::import("json");
            return json_module.attr("loads")(json_str);
        },
        "回测完成后，返回最后一天交易记录，以及需要延迟的买入和卖出延迟请求")

        DEF_PICKLE(System);

    //--------------------------------------------------------------------------------------
    // 递归组合重构：聚合交易系统（组合回测）
    py::class_<MultiSystem, System, std::shared_ptr<MultiSystem>>(m, "MultiSystem", py::dynamic_attr(),
      R"(聚合交易系统（组合回测）。持有多个子系统（单证券或嵌套聚合），在开盘/收盘阶段分别驱动并汇总下单。
每个子系统拥有各自独立的虚拟账户（模式 A 影子账户 / 模式 B 由父分配额度），父系统在自身账户上统一分配与下单。)")
      .def(py::init<>())
      .def(py::init<const string&>(), py::arg("name") = "MultiSystem")
      .def(py::init<const SystemList&, const string&>(), py::arg("sys_list"),
           py::arg("name") = "MultiSystem")
      .def("add", &MultiSystem::add, py::arg("sys"), "添加子系统（含循环引用检测）")
      .def("get_system_list", &MultiSystem::getSystemList, "获取子系统列表")
      .def("run", py::overload_cast<const KData&, bool, bool>(&MultiSystem::run), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false,
           R"(run(self, kdata, reset=True, reset_all=False)

    组合回测入口。kdata 作为对齐时间轴，遍历每个交易日分别驱动所有子系统（开盘/收盘）并汇总下单。
    各子系统拥有独立虚拟账户，父系统在自身账户上统一分配与下单。

    :param KData kdata: 对齐的时间轴（应覆盖各子系统的交易日）)")
      .def("run", py::overload_cast<const KQuery&, bool, bool>(&MultiSystem::run), py::arg("query"),
           py::arg("reset") = true, py::arg("reset_all") = false,
           R"(run(self, query, reset=True, reset_all=False)

    [master 兼容] 以市场交易日历为驱动轴运行，等价 master 的 Portfolio.run(query)。

    与 run(kdata) 的区别：驱动轴不再取入参 KData 自带日期，而是取交易日历
    （已注入的固定时间轴优先，否则为 StockManager.get_trading_calendar(query)）。
    价格与 ktype 上下文取自参考标的 KData（自身标的 → 首个子系统标的 → 日历基准指数）。

    :param Query query: 查询条件
    :param bool reset: 运行前是否复位
    :param bool reset_all: 运行前是否强制全量复位)")
      .def("runMoment", &MultiSystem::runMoment, py::arg("datetime"),
           R"(runMoment(self, datetime)

    在指定时刻执行一步，分别驱动所有子系统（开盘/收盘），并汇总交易与建议。

    :param Datetime datetime: 指定的日期
    :rtype: MomentResult)")
      .def("runMomentOnOpen", &MultiSystem::runMomentOnOpen, py::arg("datetime"))
      .def("runMomentOnClose", &MultiSystem::runMomentOnClose, py::arg("datetime"))
      .def("ready_for_run", &MultiSystem::readyForRun)
      .def("set_mode", &MultiSystem::setMode, py::arg("mode"), "设置运行模式：A（信号汇总）/ B（资金划拨）")
      .def_property_readonly("mode", &MultiSystem::getMode, "当前运行模式（A/B）")
      .def("set_sub_init_cash", &MultiSystem::setSubInitCash, py::arg("cash"),
           "设置子系统影子账户初始资金（模式 A 固定值 / 模式 B 初始额度）")
      .def("set_adjust_cycle", &MultiSystem::setAdjustCycle, py::arg("days"),
           "设置调仓周期（天），<=1 表示每个收盘日都再平衡")
      .def("set_axis_mode", &MultiSystem::setAxisMode, py::arg("mode"),
           R"(set_axis_mode(self, mode)

    设置驱动时间轴模式："kdata"（默认，以 run(kdata) 入参 KData 自带日期为驱动轴）
    或 "calendar"（以 set_date_axis 注入的固定日期表为驱动轴）。非法取值告警并回退 "kdata"。

    :param str mode: "kdata" / "calendar")")
      .def("get_axis_mode", &MultiSystem::getAxisMode, "获取驱动时间轴模式")
      .def(
        "set_date_axis",
        [](MultiSystem& ms, const py::object& dates) { ms.setDateAxis(toDatetimeList(dates)); },
        py::arg("dates"),
        "设置固定日期表（接受 list/tuple/DatetimeList；仅 axis_mode == \"calendar\" 时作为驱动轴；"
        "空表回退 kdata 轴并告警）")
      .def("get_date_axis", &MultiSystem::getDateAxis, "获取固定日期表")
      .def("clear_date_axis", &MultiSystem::clearDateAxis, "清空固定日期表")
      .def(
        "set_adjust_dates",
        [](MultiSystem& ms, const py::object& dates) { ms.setAdjustDates(toDatetimeList(dates)); },
        py::arg("dates"),
        R"(set_adjust_dates(self, dates)

    设置外部调仓日表（非空时优先作为调仓日判据，否则回退 set_adjust_cycle 的计数判定）。
    传入日期统一归一化为当日零点后存入，仅命中表内日期才执行再平衡。

    :param dates: Datetime 序列（list/tuple/DatetimeList 均可）)")
      .def(
        "get_adjust_dates",
        [](const MultiSystem& ms) {
            // 返回 list（DatetimeList）而非 C++ std::set：与 get_date_axis 一致，
            // 且规避 Python 侧对 Datetime 无 __hash__ 时 set 转换失败的问题
            const auto& dates = ms.getAdjustDates();
            return DatetimeList(dates.begin(), dates.end());
        },
        "获取外部调仓日表（已归一化为当日零点）")
      .def("clear_adjust_dates", &MultiSystem::clearAdjustDates,
           "清空外部调仓日表（回退调仓周期计数判定）")
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

    [静态] 在给定交易日轴上计算调仓日集合（纯函数，可用于预览调仓节奏）。

    :param dates: 已排序的交易日序列（list/tuple/DatetimeList）
    :param str mode: "week" / "month" / "quarter" / "year"（其余取值返回空）
    :param int adjust_cycle: 周期内第 N 日（<=0 视为 1）
    :param bool delay_to_trading_day: 目标日非交易日时是否顺延至当周期内首个交易日
    :rtype: DatetimeList)")
      .def("set_trade_on_close", &MultiSystem::setTradeOnClose, py::arg("on_close"),
           "设置是否在收盘阶段执行调仓下单")
      .def("set_adjust_mode", &MultiSystem::setAdjustMode, py::arg("mode"),
           R"(set_adjust_mode(self, mode)

    设置调仓模式（承接 master PF 的 adjust_mode）：
      - "query" / "day"（默认）：沿用 set_adjust_cycle 的「每 N 个收盘日」计数判定；
      - "week" / "month" / "quarter" / "year"：在驱动轴上按「周期内第 adjust_cycle 日」展开调仓日表。
    非法取值告警并回退 "query"。

    :param str mode: "query" / "day" / "week" / "month" / "quarter" / "year")")
      .def("get_adjust_mode", &MultiSystem::getAdjustMode, "获取调仓模式")
      .def("set_delay_to_trading_day", &MultiSystem::setDelayToTradingDay, py::arg("delay"),
           "设置调仓日非交易日时是否顺延至当周期内首个交易日（仅 week/month/quarter/year 展开时生效）")
      .def("get_delay_to_trading_day", &MultiSystem::getDelayToTradingDay,
           "获取调仓日是否顺延至交易日")
      .def("set_se", &MultiSystem::setSE, py::arg("se"), "设置交易对象选择器（可选，仅调仓日选股过滤）")
      .def_property_readonly("se", &MultiSystem::getSE, "交易对象选择器")
      .def("set_sell_at_not_selected", &MultiSystem::setSellAtNotSelected, py::arg("on"),
           "设置未选中子系统是否强制清仓（需设置 SE）")
      .def("get_adjust_turnover", &MultiSystem::getAdjustTurnover,
           "获取各调仓日的换手率（(日期, 成交金额/总资产) 列表）")
      // 与 PySystem 的 set_* 一致：设置 Python 自定义部件时持 GIL 并 release() 保活，
      // 防止 Python 侧部件（如自定义 MM/SG）被提前 GC 导致 C++ 侧持悬垂指针（use-after-free）。
      .def_property("tm", &MultiSystem::getTM,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setTM(o.cast<TradeManagerPtr>());
                        tmp.release();
                    },
                    "关联的交易管理实例")
      .def_property("mm", &MultiSystem::getMM,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setMM(o.cast<MMPtr>());
                        tmp.release();
                    },
                    "资金管理策略")
      .def_property("ev", &MultiSystem::getEV,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setEV(o.cast<EnvironmentPtr>());
                        tmp.release();
                    },
                    "市场环境判断策略")
      .def_property("cn", &MultiSystem::getCN,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setCN(o.cast<CNPtr>());
                        tmp.release();
                    },
                    "系统有效条件")
      .def_property("sg", &MultiSystem::getSG,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setSG(o.cast<SGPtr>());
                        tmp.release();
                    },
                    "信号指示器")
      .def_property("st", &MultiSystem::getST,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setST(o.cast<StoplossPtr>());
                        tmp.release();
                    },
                    "止损策略")
      .def_property("tp", &MultiSystem::getTP,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setTP(o.cast<StoplossPtr>());
                        tmp.release();
                    },
                    "止盈策略")
      .def_property("pg", &MultiSystem::getPG,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setPG(o.cast<PGPtr>());
                        tmp.release();
                    },
                    "盈利目标策略")
      .def_property("sp", &MultiSystem::getSP,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setSP(o.cast<SlippagePtr>());
                        tmp.release();
                    },
                    "移滑价差算法")
      .def_property("af", &MultiSystem::getAF,
                    [](MultiSystem& self, py::object o) {
                        py::gil_scoped_acquire gil;
                        auto tmp = o;
                        self.setAF(o.cast<AllocateFundsPtr>());
                        tmp.release();
                    },
                    "组合级资金分配算法（AF，承载 L1/L2/L3；仅聚合系统使用）")
      .def("clone", &MultiSystem::clone);

    //--------------------------------------------------------------------------------------
    // v5：PF 兼容层（工厂直通到 MultiSystem），保持 master 调用方式不变
    // （见 docs/design/pf_af_compat/design.md §4；返回类型由 PortfolioPtr 变为 MultiSystem）
    m.def(
      "PF_Simple", &PF_Simple, py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
      py::arg("af") = AF_EqualWeight(), py::arg("adjust_cycle") = 1,
      py::arg("adjust_mode") = "query", py::arg("delay_to_trading_day") = true,
      py::keep_alive<0, 1>(), py::keep_alive<0, 2>(), py::keep_alive<0, 3>(),
      R"(PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    创建一个多标的、单系统策略的投资组合（v5：返回 MultiSystem，语义为模式 B 额度划拨）

    调仓模式 adjust_mode 说明：
    - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype
      决定周期间隔；
    - "day" 模式，adjust_cycle 为调仓间隔天数
    - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle
      为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时
      如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日
      将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月
      的第一个交易日。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 组合级资金分配算法（AF，承载 L1/L2/L3）
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :rtype: MultiSystem)");

    m.def(
      "PF_WithoutAF", &PF_WithoutAF, py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
      py::arg("adjust_cycle") = 1, py::arg("adjust_mode") = "query",
      py::arg("delay_to_trading_day") = true, py::arg("trade_on_close") = true,
      py::arg("sys_use_self_tm") = false, py::arg("sell_at_not_selected") = false,
      py::keep_alive<0, 1>(), py::keep_alive<0, 2>(),
      R"(PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    创建无资金分配算法的投资组合（v5：返回 MultiSystem，语义为模式 A 信号汇总）

    调仓模式 adjust_mode 说明：
    - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype
      决定周期间隔；
    - "day" 模式，adjust_cycle 为调仓间隔天数
    - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle
      为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时
      如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true时，如果当日不是交易日
      将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月
      的第一个交易日。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :param bool trade_on_close: 交易是否在收盘时进行
    :param bool sys_use_self_tm: 原型系统使用自身附带的tm进行计算（v5 忽略并告警）
    :param bool sell_at_not_selected: 调仓日未选中的股票是否强制卖出
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

  创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出），  系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
  和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
  且没有正确结果的时候，可能是未设置tm、sg、mm。进行回测时，使用 run 方法，如::
    
        #创建模拟交易账户进行回测，初始资金30万
        my_tm = crtTM(init_cash = 300000)

        #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
        my_sg = SG_Flex(EMA(C, n=5), slow_n=10)

        #固定每次买入1000股
        my_mm = MM_FixedCount(1000)

        #创建交易系统并运行
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))
    
    :param TradeManager tm: 交易管理实例 
    :param MoneyManager mm: 资金管理策略
    :param EnvironmentBase ev: 市场环境判断策略
    :param ConditionBase cn: 系统有效条件
    :param SignalBase sg: 信号指示器
    :param StoplossBase st: 止损策略
    :param StoplossBase tp: 止盈策略
    :param ProfitGoalBase pg: 盈利目标策略
    :param SlippageBase sp: 移滑价差算法
    :return: system实例)");

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

  创建滚动寻优系统，当输入的候选系统列表中仅有一个候选系统时，即为滚动系统

  :param sequence sys_list: 候选系统列表
  :param TradeManager tm: 交易账户
  :param int train_len: 滚动评估系统绩效时使用的数据长度
  :param int test_len: 使用在 train_len 中选出的最优系统执行的数据长度
  :param SelectorBase se: 寻优选择器，默认为按“帐户平均年收益率%”最大选择
  :param TradeManager train_tm: 滚动评估时使用的交易账户, 为None时, 使用 tm 的拷贝进行评估)");
}