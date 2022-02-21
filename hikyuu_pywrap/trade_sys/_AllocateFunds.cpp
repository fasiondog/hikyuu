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

    // SystemWeightList::const_reference (SystemWeightList::*SystemWeightList_at)(
    //   SystemWeightList::size_type) const = &SystemWeightList::at;
    // void (SystemWeightList::*append)(const SystemWeight&) = &SystemWeightList::push_back;
    // class_<SystemWeightList>("SystemWeightList")
    //   .def("__iter__", iterator<SystemWeightList>())
    //   .def("size", &SystemWeightList::size)
    //   .def("__len__", &SystemWeightList::size)
    //   .def("get", SystemWeightList_at, return_value_policy<copy_const_reference>())
    //   .def("append", append);

    class_<AllocateFundsBaseWrap, boost::noncopyable>("AllocateFundsBase", init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))
      .add_property("name", make_function(af_get_name, return_value_policy<copy_const_reference>()),
                    af_set_name, "算法组件名称")
      .def("get_param", &AllocateFundsBase::getParam<boost::any>)
      .def("set_param", &AllocateFundsBase::setParam<object>)
      .def("have_param", &AllocateFundsBase::haveParam)

      .def("reset", &AllocateFundsBase::reset)
      .def("clone", &AllocateFundsBase::clone)
      .def("_reset", &AllocateFundsBase::_reset, &AllocateFundsBaseWrap::default_reset)
      .def("_clone", pure_virtual(&AllocateFundsBase::_clone))
      .def("_allocate_weight", pure_virtual(&AllocateFundsBase::_allocateWeight))
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<AllocateFundsBase>())
#endif
      ;

    register_ptr_to_python<AFPtr>();

    def("AF_EqualWeight", AF_EqualWeight);
    def("AF_FixedWeight", AF_FixedWeight);
}
