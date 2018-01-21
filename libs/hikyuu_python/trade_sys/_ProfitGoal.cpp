/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/profitgoal/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class ProfitGoalWrap : public ProfitGoalBase, public wrapper<ProfitGoalBase> {
public:
    ProfitGoalWrap(): ProfitGoalBase() {}
    ProfitGoalWrap(const string& name): ProfitGoalBase(name) {}
    virtual ~ProfitGoalWrap() {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            ProfitGoalBase::_reset();
        }
    }

    void default_reset() {
        this->ProfitGoalBase::_reset();
    }

    ProfitGoalPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    void buyNotify(const TradeRecord& tr) {
        if (override buy_notify = this->get_override("buyNotify")) {
            buy_notify(tr);
            return;
        }

        this->ProfitGoalBase::buyNotify(tr);
    }

    void default_buyNotify(const TradeRecord& tr) {
        this->ProfitGoalBase::buyNotify(tr);
    }

    void sellNotify(const TradeRecord& tr) {
        if (override sell_notify = this->get_override("sellNotify")) {
            sell_notify(tr);
            return;
        }

        this->ProfitGoalBase::sellNotify(tr);
    }

    void default_sellNotify(const TradeRecord& tr) {
        this->ProfitGoalBase::sellNotify(tr);
    }

    price_t getGoal(const Datetime& datetime, price_t price) {
        return this->get_override("getGoal")(datetime, price);
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

string (ProfitGoalBase::*pg_get_name)() const = &ProfitGoalBase::name;
void (ProfitGoalBase::*pg_set_name)(const string&) = &ProfitGoalBase::name;

void export_ProfitGoal() {
    class_<ProfitGoalWrap, boost::noncopyable>("ProfitGoalBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", pg_get_name, pg_set_name)
            .def("getParam", &ProfitGoalBase::getParam<boost::any>)
            .def("setParam", &ProfitGoalBase::setParam<object>)
            .def("setTM", &ProfitGoalBase::setTM)
            .def("setTO", &ProfitGoalBase::setTO)
            .def("getTM", &ProfitGoalBase::getTM)
            .def("getTO", &ProfitGoalBase::getTO)
            .def("buyNotify", &ProfitGoalBase::buyNotify,
                              &ProfitGoalWrap::default_buyNotify)
            .def("sellNotify", &ProfitGoalBase::sellNotify,
                               &ProfitGoalWrap::default_sellNotify)
            .def("getGoal", pure_virtual(&ProfitGoalBase::getGoal))
            .def("getShortGoal", &ProfitGoalBase::getShortGoal,
                    & ProfitGoalWrap::default_getShortGoal)
            .def("reset", &ProfitGoalBase::reset)
            .def("clone", &ProfitGoalBase::clone)
            .def("_calculate", pure_virtual(&ProfitGoalBase::_calculate))
            .def("_reset", &ProfitGoalBase::_reset, &ProfitGoalWrap::default_reset)
            .def("_clone", pure_virtual(&ProfitGoalBase::_clone))
            ;
    register_ptr_to_python<ProfitGoalPtr>();

    def("PG_NoGoal", PG_NoGoal);
    def("PG_FixedPercent", PG_FixedPercent, (arg("p") = 0.2));
    def("PG_FixedHoldDays", PG_FixedHoldDays, (arg("days") = 5));
}





