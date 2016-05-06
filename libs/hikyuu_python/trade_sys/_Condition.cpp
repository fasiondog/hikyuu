/*
 * _Condition.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */


#include <boost/python.hpp>
#include <hikyuu/trade_sys/condition/ConditionBase.h>

using namespace boost::python;
using namespace hku;

class ConditionWrap : public ConditionBase, public wrapper<ConditionBase> {
public:
    ConditionWrap(): ConditionBase() {}
    ConditionWrap(const string& name): ConditionBase(name) {}

    bool isValid(const Datetime& datetime) {
        return this->get_override("isValid")(datetime);
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    void _reset() {
        this->get_override("_reset")();
    }

    ConditionPtr _clone() {
        return this->get_override("_clone")();
    }
};


void export_Condition() {
    class_<ConditionWrap, boost::noncopyable>("ConditionBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&ConditionBase::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&ConditionBase::getParameter,
                            return_internal_reference<>()))
            .def("isValid", pure_virtual(&ConditionBase::isValid))
            .def("setTO", &ConditionBase::setTO)
            .def("reset", &ConditionBase::reset)
            .def("clone", &ConditionBase::clone)
            .def("_calculate", pure_virtual(&ConditionBase::_calculate))
            .def("_reset", pure_virtual(&ConditionBase::_reset))
            .def("_clone", pure_virtual(&ConditionBase::_clone))
            ;
    register_ptr_to_python<ConditionPtr>();
}
