/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/stoploss/build_in.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class StoplossWrap : public StoplossBase, public wrapper<StoplossBase> {
public:
    StoplossWrap(const string& name): StoplossBase(name) {}
    virtual ~StoplossWrap() {}

    void _reset() {
        this->get_override("_reset")();
    }

    StoplossPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate");
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

BOOST_PYTHON_FUNCTION_OVERLOADS(SAFETYLOSS_ST_overload, Saftyloss_ST, 0, 3);

void export_Stoploss() {
    class_<StoplossWrap, boost::noncopyable>("StoplossBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&StoplossBase::name,
                            return_value_policy<copy_const_reference>()),
                    &StoplossBase::setName)
            .add_property("params",
                    make_function(&StoplossBase::getParameter,
                            return_internal_reference<>()))
            .def("setTM", &StoplossBase::setTM)
            .def("setTO", &StoplossBase::setTO)
            .def("getPrice", pure_virtual(&StoplossBase::getPrice))
            .def("getShortPrice", &StoplossBase::getShortPrice,
                    &StoplossWrap::default_getShortPrice)
            .def("reset", &StoplossBase::reset)
            .def("clone", &StoplossBase::clone)
            .def("_calculate", pure_virtual(&StoplossBase::_calculate))
            .def("_reset", pure_virtual(&StoplossBase::_reset))
            .def("_clone", pure_virtual(&StoplossBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<StoplossBase>())
#endif
            ;

    register_ptr_to_python<StoplossPtr>();

    def("FixedPercent_SL", FixedPercent_SL);
    def("Saftyloss_ST", Saftyloss_ST, SAFETYLOSS_ST_overload());
}


