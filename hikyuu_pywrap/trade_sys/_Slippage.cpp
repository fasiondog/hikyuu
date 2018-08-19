/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SlippageWrap : public SlippageBase, public wrapper<SlippageBase> {
public:
    SlippageWrap() : SlippageBase() {}
    SlippageWrap(const string& name): SlippageBase(name) {}
    virtual ~SlippageWrap() {}

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            SlippageBase::_reset();
        }
    }

    void default_reset() {
        this->SlippageBase::_reset();
    }

    SlippagePtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate");
    }

    price_t getRealBuyPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getRealBuyPrice")(datetime, price);
    }

    price_t getRealSellPrice(const Datetime& datetime, price_t price) {
        return this->get_override("getRealSellPrice")(datetime, price);
    }
};


string (SlippageBase::*sp_get_name)() const = &SlippageBase::name;
void (SlippageBase::*sp_set_name)(const string&) = &SlippageBase::name;


void export_Slippage() {
    class_<SlippageWrap, boost::noncopyable>("SlippageBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", sp_get_name, sp_set_name)
            .def("getParam", &SlippageBase::getParam<boost::any>)
            .def("setParam", &SlippageBase::setParam<object>)
            .def("setTO", &SlippageBase::setTO)
            .def("getTO", &SlippageBase::getTO)
            .def("getRealBuyPrice", pure_virtual(&SlippageBase::getRealBuyPrice))
            .def("getRealSellPrice", pure_virtual(&SlippageBase::getRealSellPrice))
            .def("reset", &SlippageBase::reset)
            .def("clone", &SlippageBase::clone)
            .def("_calculate", pure_virtual(&SlippageBase::_calculate))
            .def("_reset", &SlippageBase::_reset, &SlippageWrap::default_reset)
            .def("_clone", pure_virtual(&SlippageBase::_clone))
            ;
    register_ptr_to_python<SlippagePtr>();

    def("SL_FixedPercent", SP_FixedPercent, (arg("p") = 0.001));
    def("SL_FixedValue", SP_FixedValue, (arg("value") = 0.01));
}





