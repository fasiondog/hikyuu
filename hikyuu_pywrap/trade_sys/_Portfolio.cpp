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

void export_Portfolio() {
    class_<Portfolio>("Portfolio", R"(实现多标的、多策略的投资组合)", init<>())
      .def(init<const string&>())
      .def(init<const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", make_function(pf_get_name, return_value_policy<copy_const_reference>()),
                    pf_set_name, "名称")
      .add_property(
        "query", make_function(&Portfolio::getQuery, return_value_policy<copy_const_reference>()),
        &Portfolio::setQuery, "查询条件")

      .add_property("tm", &Portfolio::getTM, &Portfolio::setTM, "设置或获取交易管理对象")
      .add_property("se", &Portfolio::getSE, &Portfolio::setSE, "设置或获取交易对象选择算法")
      .add_property("af", &Portfolio::getAF, &Portfolio::setAF, "设置或获取资产分配算法")
      .add_property(
        "proto_sys_list",
        make_function(&Portfolio::getProtoSystemList, return_value_policy<copy_const_reference>()),
        "获取原型系统列")
      .add_property(
        "real_sys_list",
        make_function(&Portfolio::getRealSystemList, return_value_policy<copy_const_reference>()),
        "由 PF 运行时设定的实际运行系统列表")

      .def("get_param", &Portfolio::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &Portfolio::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &Portfolio::haveParam, "是否存在指定参数")

      .def("reset", &Portfolio::reset, "复位操作")
      .def("clone", &Portfolio::clone, "克隆操作")

      .def("run", &Portfolio::run, (arg("query"), arg("force") = false), R"(run(self, query)
    
    运行投资组合策略。在查询条件及各组件没有变化时，PF在第二次执行时，默认不会实际进行计算。
    但由于各个组件的参数可能改变，此种情况无法自动判断是否需要重计算，可以手工指定进行强制计算。
        
    :param Query query: 查询条件
    :param bool force: 强制重新计算)")

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
