/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class AllocateFundsBaseWrap : public AllocateFundsBase, public wrapper<AllocateFundsBase> {
public:
    AllocateFundsBaseWrap(): AllocateFundsBase() {}
    AllocateFundsBaseWrap(const string& name): AllocateFundsBase(name) {}
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

    SystemWeightList _allocateWeight(const Datetime& date,
                                    const SystemList& se_list) {
        return this->get_override("_allocateWeight")(date, se_list);
    }

    AFPtr _clone() {
        return this->get_override("_clone")();
    }
};

string (AllocateFundsBase::*af_get_name)() const = &AllocateFundsBase::name;
void (AllocateFundsBase::*af_set_name)(const string&) = &AllocateFundsBase::name;

void export_AllocateFunds() {
    class_<SystemWeight>("SystemWeight", init<>())
            .def(init<const SystemPtr&, price_t>())
            .def(self_ns::str(self))
            .def_readwrite("sys", &SystemWeight::m_sys)
            .def_readwrite("weight", &SystemWeight::m_weight)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<SystemWeight>())
#endif
            ;

    SystemWeightList::const_reference (SystemWeightList::*SystemWeightList_at)(SystemWeightList::size_type) const = &SystemWeightList::at;
    void (SystemWeightList::*append)(const SystemWeight&) = &SystemWeightList::push_back;
    class_<SystemWeightList>("SystemWeightList")
            .def("__iter__", iterator<SystemWeightList>())
            .def("size", &SystemWeightList::size)
            .def("__len__", &SystemWeightList::size)
            .def("get", SystemWeightList_at, return_value_policy<copy_const_reference>())
            .def("append", append)
            ;


    class_<AllocateFundsBaseWrap, boost::noncopyable>("AllocateFundsBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", af_get_name, af_set_name)
            .def("getParam", &AllocateFundsBase::getParam<boost::any>)
            .def("setParam", &AllocateFundsBase::setParam<object>)

            .def("reset", &AllocateFundsBase::reset)
            .def("clone", &AllocateFundsBase::clone)
            .def("_reset", &AllocateFundsBase::_reset, &AllocateFundsBaseWrap::default_reset)
            .def("_clone", pure_virtual(&AllocateFundsBase::_clone))
            .def("_allocateWeight", pure_virtual(&AllocateFundsBase::_allocateWeight))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<AllocateFundsBase>())
#endif
            ;

    register_ptr_to_python<AFPtr>();

    def("AF_EqualWeight", AF_EqualWeight);

}


