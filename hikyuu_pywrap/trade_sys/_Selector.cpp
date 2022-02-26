/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/selector/build_in.h>
#include "../_Parameter.h"
#include "../pybind_utils.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SelectorWrap : public SelectorBase, public wrapper<SelectorBase> {
public:
    SelectorWrap() : SelectorBase() {}
    SelectorWrap(const string& name) : SelectorBase(name) {}
    virtual ~SelectorWrap() {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            SelectorBase::_reset();
        }
    }

    void default_reset() {
        this->SelectorBase::_reset();
    }

    SystemList getSelectedOnOpen(Datetime date) {
        return this->get_override("get_selected_on_open")(date);
    }

    SystemList getSelectedOnClose(Datetime date) {
        return this->get_override("get_selected_on_close")(date);
    }

    SelectorPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }
};

const string& (SelectorBase::*se_get_name)() const = &SelectorBase::name;
void (SelectorBase::*se_set_name)(const string&) = &SelectorBase::name;

SelectorPtr (*SE_Fixed_1)() = SE_Fixed;
SelectorPtr py_SE_Fixed(py::list stock_list, const SystemPtr& sys) {
    return SE_Fixed(python_list_to_vector<StockList>(stock_list), sys);
}

SEPtr (*SE_Signal_1)() = SE_Signal;
SelectorPtr py_SE_Signal(py::list stock_list, const SystemPtr& sys) {
    return SE_Signal(python_list_to_vector<StockList>(stock_list), sys);
}

bool py_se_add_stock_list(SelectorBase& se, py::list stock_list, const SystemPtr& protoSys) {
    return se.addStockList(python_list_to_vector<StockList>(stock_list), protoSys);
}

void export_Selector() {
    class_<SelectorWrap, boost::noncopyable>(
      "SelectorBase",
      R"(选择器策略基类，实现标的、系统策略的评估和选取算法，自定义选择器策略子类接口：

    - get_selected_on_open - 【必须】获取指定时刻开盘时选择的系统实例列表
    - get_selected_on_close - 【必须】获取指定时刻收盘时选择的系统实例列表
    - _calculate - 【必须】计算接口
    - _reset - 【可选】重置私有属性
    - _clone - 【必须】克隆接口)",
      init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .add_property("name", make_function(se_get_name, return_value_policy<copy_const_reference>()),
                    se_set_name, "算法名称")
      .add_property("proto_sys_list",
                    make_function(&SelectorBase::getProtoSystemList,
                                  return_value_policy<copy_const_reference>()),
                    "原型系统列表")
      .add_property("real_sys_list",
                    make_function(&SelectorBase::getRealSystemList,
                                  return_value_policy<copy_const_reference>()),
                    "由 PF 运行时设定的实际运行系统列表")

      .def("get_param", &SelectorBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SelectorBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SelectorBase::haveParam, "是否存在指定参数")

      .def("reset", &SelectorBase::reset, "复位操作")
      .def("clone", &SelectorBase::clone, "克隆操作")
      .def("clear", &SelectorBase::clear)

      .def("add_stock", &SelectorBase::addStock, (arg("stock"), arg("sys")),
           R"(add_stock(self, stock, sys)

    加入初始标的及其对应的系统策略原型

    :param Stock stock: 加入的初始标的
    :param System sys: 系统策略原型)")

      .def("add_stock_list", py_se_add_stock_list, (arg("stk_list"), arg("sys")),
           R"(add_stock_list(self, stk_list, sys)

    加入初始标的列表及其系统策略原型

    :param StockList stk_list: 加入的初始标的列表
    :param System sys: 系统策略原型)")

      .def("_reset", &SelectorBase::_reset, &SelectorWrap::default_reset, "子类复位操作实现")
      .def("_clone", pure_virtual(&SelectorBase::_clone), "子类克隆操作实现接口")
      .def("_calculate", pure_virtual(&SelectorBase::_calculate), "【重载接口】子类计算接口")

      .def("get_selected_on_open", pure_virtual(&SelectorBase::getSelectedOnOpen),
           R"(get_selected_on_open(self, datetime)

    【重载接口】获取指定时刻开盘时选取的系统实例

    :param Datetime datetime: 指定时刻
    :return: 选取的系统实例列表
    :rtype: SystemList)")

      .def("get_selected_on_close", pure_virtual(&SelectorBase::getSelectedOnClose),
           R"(get_selected_on_close(self, datetime)

    【重载接口】获取指定时刻收盘时选取的系统实例

    :param Datetime datetime: 指定时刻
    :return: 选取的系统实例列表
    :rtype: SystemList)")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<SelectorBase>())
#endif
      ;

    register_ptr_to_python<SelectorPtr>();

    def("SE_Fixed", SE_Fixed_1);
    def("SE_Fixed", py_SE_Fixed, R"(SE_Fixed([stk_list, sys])

    固定选择器，即始终选择初始划定的标的及其系统策略原型

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例)");

    def("SE_Signal", SE_Signal_1);
    def("SE_Signal", py_SE_Signal, R"(SE_Signal([stk_list, sys])

    信号选择器，仅依靠系统买入信号进行选中

    :param list stk_list: 初始划定的标的
    :param System sys: 系统策略原型
    :return: SE选择器实例)");
}
