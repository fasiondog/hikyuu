/*
 * _Portfolio.cpp
 *
 *  Created on: 2016年3月29日
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

void PyPortfolio::_runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    PYBIND11_OVERLOAD(void, PyPortfolio, _runMoment, date, nextCycle, adjust);
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
                                                     R"(实现多标的、多策略的投资组合)")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const string&, const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())

      .def("__str__", &Portfolio::str)
      .def("__repr__", &Portfolio::str)

      .def_property("name", py::overload_cast<>(&Portfolio::name, py::const_),
                    py::overload_cast<const string&>(&Portfolio::name),
                    py::return_value_policy::copy, "名称")
      .def_property("query", &Portfolio::getQuery, &Portfolio::setQuery,
                    py::return_value_policy::copy, "查询条件")

      .def_property(
        "tm", &Portfolio::getTM, [](PyPortfolio& self, py::object tm) { self.set_tm(tm); },
        "设置或获取交易管理对象")
      .def_property(
        "se", &Portfolio::getSE, [](PyPortfolio& self, py::object se) { self.set_se(se); },
        "设置或获取交易对象选择算法")
      .def_property(
        "af", &Portfolio::getAF, [](PyPortfolio& self, py::object af) { self.set_af(af); },
        "设置或获取资产分配算法")
      .def_property_readonly("real_sys_list", &Portfolio::getRealSystemList,
                             py::return_value_policy::copy, "由 PF 运行时设定的实际运行系统列表")

      .def("get_param", &Portfolio::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &Portfolio::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &Portfolio::haveParam, "是否存在指定参数")

      .def("reset", &Portfolio::reset, "复位操作")
      .def("clone", &Portfolio::clone, "克隆操作")

      .def("run", &Portfolio::run, py::arg("query"), py::arg("force") = false,
           R"(run(self, query[, force=false])
    
    运行投资组合策略。在查询条件及各组件没有变化时，PF在第二次执行时，默认不会实际进行计算。
    但由于各个组件的参数可能改变，此种情况无法自动判断是否需要重计算，可以手工指定进行强制计算。
        
    :param Query query: 查询条件
    :param bool force: 强制重新计算)")

      .def(
        "last_suggestion",
        [](const Portfolio& pf) {
            json j = pf.lastSuggestion();
            std::string json_str = j.dump();
            py::module json_module = py::module::import("json");
            return json_module.attr("loads")(json_str);
        },
        "回测完成后，返回最后一天交易记录，以及需要延迟的买入和卖出延迟请求")

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

    创建一个多标的、单系统策略的投资组合

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
    :param AllocateFundsBase af: 资金分配算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日)");

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
    
    创建无资金分配算法的投资组合，所有单系统策略使用共同的 tm 管理账户

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
    :param bool sys_use_self_tm: 原型系统使用自身附带的tm进行计算
    :param bool sell_at_not_selected: 调仓日未选中的股票是否强制卖出)");
}
