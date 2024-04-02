/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/selector/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySelectorBase : public SelectorBase {
    PY_CLONE(PySelectorBase, SelectorBase)

public:
    using SelectorBase::SelectorBase;

    void _reset() override {
        PYBIND11_OVERLOAD(void, SelectorBase, _reset, );
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, SelectorBase, _calculate, );
    }

    SystemWeightList getSelected(Datetime date) override {
        // PYBIND11_OVERLOAD_PURE_NAME(SystemWeightList, SelectorBase, "get_selected", getSelected,
        //                             date);
        auto self = py::cast(this);
        py::sequence py_ret = self.attr("get_selected")(date);
        auto c_ret = python_list_to_vector<SystemWeight>(py_ret);
        return c_ret;
    }

    bool isMatchAF(const AFPtr& af) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, SelectorBase, "is_match_af", isMatchAF, af);
    }
};

void export_Selector(py::module& m) {
    py::class_<SystemWeight>(m, "SystemWeight",
                             "系统权重系数结构，在资产分配时，指定对应系统的资产占比系数")
      .def(py::init<>())
      .def(py::init<const SystemPtr&, price_t>())
      .def("__str__", to_py_str<SystemWeight>)
      .def("__repr__", to_py_str<SystemWeight>)
      .def_readwrite("sys", &SystemWeight::sys, "对应的 System 实例")
      .def_readwrite("weight", &SystemWeight::weight)

        DEF_PICKLE(SystemWeight);

    py::class_<SelectorBase, SEPtr, PySelectorBase>(
      m, "SelectorBase",
      R"(选择器策略基类，实现标的、系统策略的评估和选取算法，自定义选择器策略子类接口：

    - get_selected - 【必须】获取指定时刻选择的系统实例列表
    - _calculate - 【必须】计算接口
    - _reset - 【可选】重置私有属性
    - _clone - 【必须】克隆接口)")

      .def(py::init<>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<SelectorBase>)
      .def("__repr__", to_py_str<SelectorBase>)

      .def_property("name", py::overload_cast<>(&SelectorBase::name, py::const_),
                    py::overload_cast<const string&>(&SelectorBase::name),
                    py::return_value_policy::copy, "算法名称")
      .def_property_readonly("proto_sys_list", &SelectorBase::getProtoSystemList,
                             py::return_value_policy::copy, "原型系统列表")
      .def_property_readonly("real_sys_list", &SelectorBase::getRealSystemList,
                             py::return_value_policy::copy, "由 PF 运行时设定的实际运行系统列表")

      .def("get_param", &SelectorBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SelectorBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SelectorBase::haveParam, "是否存在指定参数")

      .def("reset", &SelectorBase::reset, "复位操作")
      .def("clone", &SelectorBase::clone, "克隆操作")
      .def("remove_all", &SelectorBase::removeAll, "清除所有已加入的原型系统")

      .def("add_stock", &SelectorBase::addStock, py::arg("stock"), py::arg("sys"),
           R"(add_stock(self, stock, sys)

    加入初始标的及其对应的系统策略原型

    :param Stock stock: 加入的初始标的
    :param System sys: 系统策略原型)")

      .def(
        "add_stock_list",
        [](SelectorBase& self, py::sequence stk_list, const SYSPtr& sys) {
            self.addStockList(python_list_to_vector<Stock>(stk_list), sys);
        },
        py::arg("stk_list"), py::arg("sys"),
        R"(add_stock_list(self, stk_list, sys)

    加入初始标的列表及其系统策略原型

    :param StockList stk_list: 加入的初始标的列表
    :param System sys: 系统策略原型)")

      .def("get_proto_sys_list", &SelectorBase::getProtoSystemList, py::return_value_policy::copy)
      .def("get_real_sys_list", &SelectorBase::getRealSystemList, py::return_value_policy::copy)
      .def("calculate", &SelectorBase::calculate)

      .def("_reset", &SelectorBase::_reset, "子类复位操作实现")
      .def("_calculate", &SelectorBase::_calculate, "【重载接口】子类计算接口")

      .def("is_match_af", &SelectorBase::isMatchAF, R"(is_match_af(self)

    【重载接口】判断是否和 AF 匹配

    :param AllocateFundsBase af: 资产分配算法)")

      .def("get_selected", &SelectorBase::getSelected,
           R"(get_selected(self, datetime)

    【重载接口】获取指定时刻选取的系统实例

    :param Datetime datetime: 指定时刻
    :return: 选取的系统实例列表
    :rtype: SystemList)")

        DEF_PICKLE(SEPtr);

    m.def("SE_Fixed", py::overload_cast<>(SE_Fixed));
    m.def("SE_Fixed", py::overload_cast<const StockList&, const SystemPtr&>(SE_Fixed),
          R"(SE_Fixed([stk_list, sys])

    固定选择器，即始终选择初始划定的标的及其系统策略原型

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例)");

    m.def("SE_Signal", py::overload_cast<>(SE_Signal));
    m.def("SE_Signal", py::overload_cast<const StockList&, const SystemPtr&>(SE_Signal),
          R"(SE_Signal([stk_list, sys])

    信号选择器，仅依靠系统买入信号进行选中

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例)");

    m.def("SE_MultiFactor", py::overload_cast<const MFPtr&, int>(SE_MultiFactor), py::arg("mf"),
          py::arg("topn") = 10);
    m.def(
      "SE_MultiFactor",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query, int topn,
         int ic_n, int ic_rolling_n, const py::object& ref_stk, const string& mode) {
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          Stock c_ref_stk = ref_stk.is_none() ? getStock("sh000300") : ref_stk.cast<Stock>();
          return SE_MultiFactor(c_inds, c_stks, query, topn, ic_n, ic_rolling_n, c_ref_stk, mode);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("topn") = 10, py::arg("ic_n") = 5,
      py::arg("ic_rolling_n") = 120, py::arg("ref_stk") = py::none(),
      py::arg("mode") = "MF_ICIRWeight",
      R"(SE_MultiFactor

    创建基于多因子评分的选择器，两种创建方式

    - 直接指定 MF:
      :param MultiFactorBase mf: 直接指定的多因子合成算法
      :param int topn: 只选取时间截面中前 topn 个系统

    - 参数直接创建:
      :param sequense(Indicator) inds: 原始因子列表
      :param sequense(stock) stks: 计算证券列表
      :param Query query: 日期范围
      :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
      :param int ic_n: 默认 IC 对应的 N 日收益率
      :param int ic_rolling_n: IC 滚动周期
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
      )");
}
