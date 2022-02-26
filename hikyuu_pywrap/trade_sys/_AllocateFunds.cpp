/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

class AllocateFundsBaseWrap : public AllocateFundsBase, public wrapper<AllocateFundsBase> {
public:
    AllocateFundsBaseWrap() : AllocateFundsBase() {}
    AllocateFundsBaseWrap(const string& name) : AllocateFundsBase(name) {}
    virtual ~AllocateFundsBaseWrap() {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            AllocateFundsBase::_reset();
        }
    }

    void default_reset() {
        this->AllocateFundsBase::_reset();
    }

    SystemWeightList _allocateWeight(const Datetime& date, const SystemList& se_list) {
        return this->get_override("_allocate_weight")(date, se_list);
    }

    AFPtr _clone() {
        return this->get_override("_clone")();
    }
};

const string& (AllocateFundsBase::*af_get_name)() const = &AllocateFundsBase::name;
void (AllocateFundsBase::*af_set_name)(const string&) = &AllocateFundsBase::name;

void export_AllocateFunds() {
    class_<SystemWeight>("SystemWeight",
                         "系统权重系数结构，在资产分配时，指定对应系统的资产占比系数", init<>())
      .def(init<const SystemPtr&, price_t>())
      .def(self_ns::str(self))
      .add_property(
        "sys", make_function(&SystemWeight::getSYS, return_value_policy<copy_const_reference>()),
        &SystemWeight::setSYS, "对应的 System 实例")
      .add_property("weight", &SystemWeight::getWeight, &SystemWeight::setWeight,
                    "对应的权重系数，有效范围为 [0, 1]")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<SystemWeight>())
#endif
      ;

    class_<SystemWeightList>("SystemWeightList").def(vector_indexing_suite<SystemWeightList>());

    class_<AllocateFundsBaseWrap, boost::noncopyable>("AllocateFundsBase",
                                                      R"(资产分配算法基类, 子类接口：

    - _allocateWeight : 【必须】子类资产分配调整实现
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)",
                                                      init<>())

      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .add_property("name", make_function(af_get_name, return_value_policy<copy_const_reference>()),
                    af_set_name, "算法组件名称")
      .add_property(
        "query",
        make_function(&AllocateFundsBase::getQuery, return_value_policy<copy_const_reference>()),
        &AllocateFundsBase::setQuery, "设置或获取查询条件")

      .def("get_param", &AllocateFundsBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &AllocateFundsBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &AllocateFundsBase::haveParam, "是否存在指定参数")

      .def("reset", &AllocateFundsBase::reset, "复位操作")
      .def("clone", &AllocateFundsBase::clone, "克隆操作")
      .def("_reset", &AllocateFundsBase::_reset, &AllocateFundsBaseWrap::default_reset,
           "子类复位操作实现")
      .def("_clone", pure_virtual(&AllocateFundsBase::_clone), "子类克隆操作实现接口")
      .def("_allocate_weight", pure_virtual(&AllocateFundsBase::_allocateWeight),
           (arg("date"), arg("se_list")), R"(_allocate_weight(self, date, se_list)

        【重载接口】子类分配权重接口，获取实际分配资产的系统实例及其权重

        :param Datetime date: 当前时间
        :param SystemList se_list: 当前选中的系统列表
        :return: 系统权重分配信息列表
        :rtype: SystemWeightList)")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<AllocateFundsBase>())
#endif
      ;

    register_ptr_to_python<AFPtr>();

    def("AF_EqualWeight", AF_EqualWeight, R"(AF_EqualWeight()
    
    等权重资产分配，对选中的资产进行等比例分配)");

    def("AF_FixedWeight", AF_FixedWeight, (arg("weight") = 0.1), R"(AF_FixedWeight(weight)
    
    固定比例资产分配

    :param float weight:  指定的资产比例 [0, 1])");
}
