/*
 * _Portfolio.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Portfolio(py::module& m) {
    py::class_<Portfolio, PortfolioPtr>(m, "Portfolio", py::dynamic_attr(),
                                        R"(实现多标的、多策略的投资组合)")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())

      .def("__str__", to_py_str<Portfolio>)
      .def("__repr__", to_py_str<Portfolio>)

      .def_property("name", py::overload_cast<>(&Portfolio::name, py::const_),
                    py::overload_cast<const string&>(&Portfolio::name),
                    py::return_value_policy::copy, "名称")
      .def_property("query", &Portfolio::getQuery, &Portfolio::setQuery,
                    py::return_value_policy::copy, "查询条件")

      .def_property("tm", &Portfolio::getTM, &Portfolio::setTM, "设置或获取交易管理对象")
      .def_property("se", &Portfolio::getSE, &Portfolio::setSE, "设置或获取交易对象选择算法")
      .def_property("af", &Portfolio::getAF, &Portfolio::setAF, "设置或获取资产分配算法")
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

      .def("run", &Portfolio::run, py::arg("query"), py::arg("adjust_cycle") = 1,
           py::arg("force") = false,
           R"(run(self, query[, adjust_cycle=1, force=false])
    
    运行投资组合策略。在查询条件及各组件没有变化时，PF在第二次执行时，默认不会实际进行计算。
    但由于各个组件的参数可能改变，此种情况无法自动判断是否需要重计算，可以手工指定进行强制计算。
        
    :param Query query: 查询条件
    :param int adjust_cycle: 调仓周期
    :param bool force: 强制重新计算)")

        DEF_PICKLE(Portfolio);

    m.def("PF_Simple", PF_Simple, py::arg("tm") = TradeManagerPtr(), py::arg("se") = SE_Fixed(),
          py::arg("af") = AF_EqualWeight(),
          R"(PF_Simple([tm, se, af])

    创建一个多标的、单系统策略的投资组合

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 资金分配算法)");
}
