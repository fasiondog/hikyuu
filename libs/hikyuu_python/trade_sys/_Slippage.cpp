/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>

using namespace boost::python;
using namespace hku;

class SlippageWrap : public SlippageBase, public wrapper<SlippageBase> {
public:
    SlippageWrap(const string& name): SlippageBase(name) {}
    virtual ~SlippageWrap() {}

    void _reset() {
        this->get_override("_reset")();
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


void export_Slippage() {
    class_<SlippageWrap, boost::noncopyable>("SlippageBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&SlippageBase::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&SlippageBase::getParameter,
                            return_internal_reference<>()))
            .def("setTO", &SlippageBase::setTO)
            .def("getRealBuyPrice", pure_virtual(&SlippageBase::getRealBuyPrice))
            .def("getRealSellPrice", pure_virtual(&SlippageBase::getRealSellPrice))
            .def("reset", &SlippageBase::reset)
            .def("clone", &SlippageBase::clone)
            .def("_calculate", pure_virtual(&SlippageBase::_calculate))
            .def("_reset", pure_virtual(&SlippageBase::_reset))
            .def("_clone", pure_virtual(&SlippageBase::_clone))
            ;
    register_ptr_to_python<SlippagePtr>();
}





