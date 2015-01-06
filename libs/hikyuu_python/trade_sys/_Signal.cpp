/*
 * _Signal.cpp
 *
 *  Created on: 2013-3-18
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/signal/build_in.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SignalWrap : public SignalBase, public wrapper<SignalBase> {
public:
    SignalWrap(const string& name): SignalBase(name) {}

    void _reset() {
        this->get_override("_reset")();
    }

    SignalPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate");
    }
};

BOOST_PYTHON_FUNCTION_OVERLOADS(AMA_SG_overload, AMA_SG, 0, 6);

void export_Signal() {
    class_<SignalWrap, boost::noncopyable>("SignalBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&SignalBase::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&SignalBase::getParameter,
                            return_internal_reference<>()))
            .def("setTO", &SignalBase::setTO)
            .def("shouldBuy", &SignalBase::shouldBuy)
            .def("shouldSell", &SignalBase::shouldSell)
            .def("getBuySignal", &SignalBase::getBuySignal)
            .def("getSellSignal", &SignalBase::getSellSignal)
            .def("_addBuySignal", &SignalBase::_addBuySignal)
            .def("_addSellSignal", &SignalBase::_addSellSignal)
            .def("reset", &SignalBase::reset)
            .def("clone", &SignalBase::clone)
            .def("_calculate", pure_virtual(&SignalBase::_calculate))
            .def("_reset", pure_virtual(&SignalBase::_reset))
            .def("_clone", pure_virtual(&SignalBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<SignalBase>())
#endif
            ;

    register_ptr_to_python<SignalPtr>();


    def("AMA_SG", AMA_SG, AMA_SG_overload());

}


