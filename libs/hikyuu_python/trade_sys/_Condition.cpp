/*
 * _Condition.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */


#include <boost/python.hpp>
#include <hikyuu/trade_sys/condition/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class ConditionWrap : public ConditionBase, public wrapper<ConditionBase> {
public:
    ConditionWrap(): ConditionBase() {}
    ConditionWrap(const string& name): ConditionBase(name) {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            ConditionBase::_reset();
        }
    }

    void default_reset() {
        this->ConditionBase::_reset();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    ConditionPtr _clone() {
        return this->get_override("_clone")();
    }
};


string (ConditionBase::*cn_get_name)() const = &ConditionBase::name;
void (ConditionBase::*cn_set_name)(const string&) = &ConditionBase::name;


void export_Condition() {
    class_<ConditionWrap, boost::noncopyable>("ConditionBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", cn_get_name, cn_set_name)
            .def("getParam", &ConditionBase::getParam<boost::any>)
            .def("setParam", &ConditionBase::setParam<object>)
            .def("isValid", &ConditionBase::isValid)
            .def("setTO", &ConditionBase::setTO)
            .def("getTO", &ConditionBase::getTO)
            .def("setTM", &ConditionBase::setTM)
            .def("getTM", &ConditionBase::getTM)
            .def("setSG", &ConditionBase::setSG)
            .def("getSG", &ConditionBase::getSG)
            .def("reset", &ConditionBase::reset)
            .def("clone", &ConditionBase::clone)
            .def("_addValid", &ConditionBase::_addValid)
            .def("_calculate", pure_virtual(&ConditionBase::_calculate))
            .def("_reset", &ConditionBase::_reset, &ConditionWrap::default_reset)
            .def("_clone", pure_virtual(&ConditionBase::_clone))
            ;
    register_ptr_to_python<ConditionPtr>();

    def("CN_OPLine", CN_OPLine);
}
