/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

class PyAllocateFundsBase : public AllocateFundsBase {
    PY_CLONE(PyAllocateFundsBase, AllocateFundsBase)

public:
    using AllocateFundsBase::AllocateFundsBase;
    PyAllocateFundsBase(const AllocateFundsBase& base) : AllocateFundsBase(base) {}

    void _reset() override {
        PYBIND11_OVERLOAD(void, AllocateFundsBase, _reset, );
    }

    SystemWeightList _allocateWeight(const Datetime& date,
                                     const SystemWeightList& se_list) override {
        PYBIND11_OVERLOAD_PURE_NAME(SystemWeightList, AllocateFundsBase, "_allocate_weight",
                                    _allocateWeight, date, se_list);
    }
};

void export_AllocateFunds(py::module& m) {
    py::class_<AllocateFundsBase, AFPtr, PyAllocateFundsBase>(
      m, "AllocateFundsBase", py::dynamic_attr(),
      R"(资产分配算法基类, 子类接口：

    - _allocateWeight : 【必须】子类资产分配调整实现
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const AllocateFundsBase&>())
      .def("__str__", to_py_str<AllocateFundsBase>)
      .def("__repr__", to_py_str<AllocateFundsBase>)
      .def_property("name", py::overload_cast<>(&AllocateFundsBase::name, py::const_),
                    py::overload_cast<const string&>(&AllocateFundsBase::name),
                    py::return_value_policy::copy, "算法组件名称")
      .def_property("query", py::overload_cast<>(&AllocateFundsBase::getQuery, py::const_),
                    py::overload_cast<const KQuery&>(&AllocateFundsBase::setQuery),
                    py::return_value_policy::copy, "设置或获取查询条件")
      .def_property_readonly("tm", py::overload_cast<>(&AllocateFundsBase::getTM, py::const_),
                             py::return_value_policy::copy)

      .def("get_param", &AllocateFundsBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &AllocateFundsBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &AllocateFundsBase::haveParam, "是否存在指定参数")

      .def("reset", &AllocateFundsBase::reset, "复位操作")
      .def("clone", &AllocateFundsBase::clone, "克隆操作")
      .def("_reset", &AllocateFundsBase::_reset, "子类复位操作实现")
      .def("_allocate_weight", &AllocateFundsBase::_allocateWeight, py::arg("date"),
           py::arg("se_list"),
           R"(_allocate_weight(self, date, se_list)

        【重载接口】子类分配权重接口，获取实际分配资产的系统实例及其权重

        :param Datetime date: 当前时间
        :param SystemList se_list: 当前选中的系统列表
        :return: 系统权重分配信息列表
        :rtype: SystemWeightList)")

        DEF_PICKLE(AFPtr);

    m.def("AF_EqualWeight", AF_EqualWeight, R"(AF_EqualWeight()
    
    等权重资产分配，对选中的资产进行等比例分配)");

    m.def("AF_FixedWeight", AF_FixedWeight, py::arg("weight") = 0.1, R"(AF_FixedWeight(weight)
    
    固定比例资产分配

    :param float weight:  指定的资产比例 [0, 1])");

    m.def("AF_MultiFactor", AF_MultiFactor, R"(AF_MultiFactor()
      
    创建 MultiFactor 评分权重的资产分配算法实例
    )");
}
