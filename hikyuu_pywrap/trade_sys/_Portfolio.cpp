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

void (Portfolio::*pf_set_name)(const string&) = &Portfolio::name;
const string& (Portfolio::*pf_get_name)() const = &Portfolio::name;

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

      .def("run", &Portfolio::run, R"(run(self, query)
    
    运行投资组合策略
        
    :param Query query: 查询条件)")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<Portfolio>())
#endif
      ;

    register_ptr_to_python<PortfolioPtr>();

    def("PF_Simple", PF_Simple,
        (arg("tm") = TradeManagerPtr(), arg("se") = SE_Fixed(), arg("af") = AF_EqualWeight()),
        R"(PF_Simple([tm, sys, se])

    创建一个多标的、单系统策略的投资组合

    :param TradeManager tm: 交易管理
    :param System sys: 系统策略
    :param SelectorBase se: 选择器)");
}
