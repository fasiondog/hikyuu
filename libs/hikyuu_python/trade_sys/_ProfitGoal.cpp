/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>

using namespace boost::python;
using namespace hku;

class ProfitGoalWrap : public ProfitGoalBase, public wrapper<ProfitGoalBase> {
public:
    ProfitGoalWrap(const string& name): ProfitGoalBase(name) {}
    virtual ~ProfitGoalWrap() {}

    void _reset() {
        this->get_override("_reset")();
    }

    ProfitGoalPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    price_t getGoal(const Datetime& datetime, price_t price) {
        if (override getGoal = get_override("getGoal")) {
            return getGoal(datetime, price);
        }
        return ProfitGoalBase::getGoal(datetime, price);
    }

    price_t default_getGoal(const Datetime& datetime, price_t price) {
        return this->ProfitGoalBase::getGoal(datetime, price);
    }

    price_t getShortGoal(const Datetime& datetime, price_t price) {
        if (override getShortGoal = get_override("getShortGoal")) {
            return getShortGoal(datetime, price);
        }
        return ProfitGoalBase::getShortGoal(datetime, price);
    }

    price_t default_getShortGoal(const Datetime& datetime, price_t price) {
        return this->ProfitGoalBase::getShortGoal(datetime, price);
    }
};


void export_ProfitGoal() {
    class_<ProfitGoalWrap, boost::noncopyable>("ProfitGoalBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&ProfitGoalBase::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&ProfitGoalBase::getParameter,
                            return_internal_reference<>()))
            .def("setTM", &ProfitGoalBase::setTM)
            .def("setTO", &ProfitGoalBase::setTO)
            .def("getGoal", &ProfitGoalBase::getGoal,
                    &ProfitGoalWrap::default_getGoal)
            .def("getShortGoal", &ProfitGoalBase::getShortGoal,
                    & ProfitGoalWrap::default_getShortGoal)
            .def("reset", &ProfitGoalBase::reset)
            .def("clone", &ProfitGoalBase::clone)
            .def("_calculate", pure_virtual(&ProfitGoalBase::_calculate))
            .def("_reset", pure_virtual(&ProfitGoalBase::_reset))
            .def("_clone", pure_virtual(&ProfitGoalBase::_clone))
            ;
    register_ptr_to_python<ProfitGoalPtr>();
}





