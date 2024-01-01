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

    SystemList getSelectedOnOpen(Datetime date) override {
        PYBIND11_OVERLOAD_PURE_NAME(SystemList, SelectorBase, "get_selected_on_open",
                                    getSelectedOnOpen, date);
    }

    SystemList getSelectedOnClose(Datetime date) override {
        PYBIND11_OVERLOAD_PURE_NAME(SystemList, SelectorBase, "get_selected_on_close",
                                    getSelectedOnClose, date);
    }

    bool isMatchAF(const AFPtr& af) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, SelectorBase, "is_match_af", isMatchAF, af);
    }
};

void export_Selector(py::module& m) {
    py::class_<SelectorBase, SEPtr, PySelectorBase>(
      m, "SelectorBase",
      R"(选择器策略基类，实现标的、系统策略的评估和选取算法，自定义选择器策略子类接口：

    - get_selected_on_open - 【必须】获取指定时刻开盘时选择的系统实例列表
    - get_selected_on_close - 【必须】获取指定时刻收盘时选择的系统实例列表
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

      .def("add_stock_list", &SelectorBase::addStockList, py::arg("stk_list"), py::arg("sys"),
           R"(add_stock_list(self, stk_list, sys)

    加入初始标的列表及其系统策略原型

    :param StockList stk_list: 加入的初始标的列表
    :param System sys: 系统策略原型)")

      .def("_reset", &SelectorBase::_reset, "子类复位操作实现")
      .def("_calculate", &SelectorBase::_calculate, "【重载接口】子类计算接口")

      .def("is_match_af", &SelectorBase::isMatchAF, R"(is_match_af(self)

    【重载接口】判断是否和 AF 匹配

    :param AllocateFundsBase af: 资产分配算法)")

      .def("get_selected_on_open", &SelectorBase::getSelectedOnOpen,
           R"(get_selected_on_open(self, datetime)

    【重载接口】获取指定时刻开盘时选取的系统实例

    :param Datetime datetime: 指定时刻
    :return: 选取的系统实例列表
    :rtype: SystemList)")

      .def("get_selected_on_close", &SelectorBase::getSelectedOnClose,
           R"(get_selected_on_close(self, datetime)

    【重载接口】获取指定时刻收盘时选取的系统实例

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
}
