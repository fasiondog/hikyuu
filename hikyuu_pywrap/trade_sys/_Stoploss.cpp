/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/stoploss/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class StoplossWrap : public StoplossBase, public wrapper<StoplossBase> {
public:
    StoplossWrap(): StoplossBase() {}
    StoplossWrap(const string& name): StoplossBase(name) {}
    virtual ~StoplossWrap() {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            StoplossBase::_reset();
        }
    }

    void default_reset() {
        this->StoplossBase::_reset();
    }

    StoplossPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }

    price_t getPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getPrice")(datetime, price);
    }

    price_t getShortPrice(const Datetime& datetime, price_t price) {
        if (override getShortPrice = get_override("getShortPrice")) {
            return getShortPrice(datetime, price);
        }
        return StoplossBase::getShortPrice(datetime, price);
    }

    price_t default_getShortPrice(const Datetime& datetime, price_t price) {
        return this->StoplossBase::getShortPrice(datetime, price);
    }
};

string (StoplossBase::*st_get_name)() const = &StoplossBase::name;
void (StoplossBase::*st_set_name)(const string&) = &StoplossBase::name;

void export_Stoploss() {
    class_<StoplossWrap, boost::noncopyable>("StoplossBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", st_get_name, st_set_name)
            .def("getParam", &StoplossBase::getParam<boost::any>)
            .def("setParam", &StoplossBase::setParam<object>)
            .def("setTM", &StoplossBase::setTM)
            .def("getTM", &StoplossBase::getTM)
            .def("setTO", &StoplossBase::setTO)
            .def("getTO", &StoplossBase::getTO)
            .def("getPrice", pure_virtual(&StoplossBase::getPrice))
            .def("getShortPrice", &StoplossBase::getShortPrice,
                    &StoplossWrap::default_getShortPrice)
            .def("reset", &StoplossBase::reset)
            .def("clone", &StoplossBase::clone)
            .def("_calculate", pure_virtual(&StoplossBase::_calculate))
            .def("_reset", &StoplossBase::_reset, &StoplossWrap::default_reset)
            .def("_clone", pure_virtual(&StoplossBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<StoplossBase>())
#endif
            ;

    register_ptr_to_python<StoplossPtr>();

    def("ST_FixedPercent", ST_FixedPercent, (arg("p") = 0.03));
    def("ST_Indicator", ST_Indicator, (arg("op"), arg("kpart") = "CLOSE"));
    def("ST_Saftyloss", ST_Saftyloss,
            (arg("n1") = 10, arg("n2") = 3, arg("p") = 2.0));
}


