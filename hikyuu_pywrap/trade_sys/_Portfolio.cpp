/*
 * _Portfolio.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;
namespace py = boost::python;

void (Portfolio::*pf_set_name)(const string&) = &Portfolio::name;
const string& (Portfolio::*pf_get_name)() const = &Portfolio::name;

FundsRecord (Portfolio::*getPortfolioFunds_1)(KQuery::KType) const = &Portfolio::getFunds;
FundsRecord (Portfolio::*getPortfolioFunds_2)(const Datetime&,
                                              KQuery::KType) = &Portfolio::getFunds;

PriceList (Portfolio::*getPFFundsCurve_1)(const DatetimeList&,
                                          KQuery::KType) = &Portfolio::getFundsCurve;
PriceList (Portfolio::*getPFFundsCurve_2)() = &Portfolio::getFundsCurve;

PriceList (Portfolio::*getPFProfitCurve_1)(const DatetimeList&,
                                           KQuery::KType ktype) = &Portfolio::getProfitCurve;
PriceList (Portfolio::*getPFProfitCurve_2)() = &Portfolio::getProfitCurve;

py::list get_real_sys_list(const Portfolio& pf) {
    return vector_to_py_list<SystemList>(pf.getRealSystemList());
}

py::list get_proto_sys_list(const Portfolio& pf) {
    return vector_to_py_list<SystemList>(pf.getProtoSystemList());
}

void export_Portfolio() {
    class_<Portfolio>("Portfolio", R"(实现多标的、多策略的投资组合)", init<>())
      .def(init<const string&>())
      .def(init<const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def("get_param", &Portfolio::getParam<boost::any>)
      .def("set_param", &Portfolio::setParam<object>)
      .def("have_param", &Portfolio::haveParam)

      .add_property("name", make_function(pf_get_name, return_value_policy<copy_const_reference>()),
                    pf_set_name, "名称")
      .add_property("tm", &Portfolio::getTM, &Portfolio::setTM, "设置或获取交易管理对象")
      .add_property("se", &Portfolio::getSE, &Portfolio::setSE, "设置或获取交易对象选择算法")

      .def("reset", &Portfolio::reset)
      .def("clone", &Portfolio::clone)

      //.def("readyForRun", &Portfolio::readyForRun)
      //.def("runMoment", &Portfolio::runMoment)

      .def("run", &Portfolio::run, (arg("query"), arg("force") = false), R"(run(self, query)
    
    运行投资组合策略。在查询条件及各组件没有变化时，PF在第二次执行时，默认不会实际进行计算。
    但由于各个组件的参数可能改变，此种情况无法自动判断是否需要重计算，可以手工指定进行强制计算。
        
    :param Query query: 查询条件
    :param bool force: 强制重新计算)")

      .def("get_real_sys_list", get_real_sys_list, "获取实际运行的子账户系统列表")
      .def("get_proto_sys_list", get_proto_sys_list, "获取原型系统列表")

      .def("get_funds", getPortfolioFunds_1, (arg("ktype") = KQuery::DAY))
      .def("get_funds", getPortfolioFunds_2, (arg("datetime"), arg("ktype") = KQuery::DAY),
           R"(get_funds(self, [datetime, ktype = Query.DAY])

    获取指定时刻的资产市值详情

    :param Datetime datetime:  指定时刻
    :param Query.KType ktype: K线类型
    :rtype: FundsRecord)")

      .def("get_funds_curve", getPFFundsCurve_1, (arg("dates"), arg("ktype") = KQuery::DAY),
           R"(get_funds_curve(self, dates[, ktype = Query.DAY])

    获取资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 资产净值列表
    :rtype: PriceList)")

      .def("get_funds_curve", getPFFundsCurve_2,
           R"(get_funds_curve(self)

    获取从账户建立日期到系统当前日期的资产净值曲线（按自然日）

    :return: 资产净值列表
    :rtype: PriceList)")

      .def("get_profit_curve", getPFProfitCurve_1, (arg("dates"), arg("ktype") = KQuery::DAY),
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    获取收益曲线，即扣除历次存入资金后的资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 收益曲线
    :rtype: PriceList)")

      .def("get_profit_curve", getPFProfitCurve_2,
           R"(get_profit_curve(self)

    获取获取从账户建立日期到系统当前日期的收益曲线

    :return: 收益曲线
    :rtype: PriceList)")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<Portfolio>())
#endif
      ;

    register_ptr_to_python<PortfolioPtr>();

    def("PF_Simple", PF_Simple,
        (arg("tm") = TradeManagerPtr(), arg("se") = SE_Fixed(), arg("af") = AF_EqualWeight()),
        R"(PF_Simple([tm, se, af])

    创建一个多标的、单系统策略的投资组合

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 资金分配算法)");
}
