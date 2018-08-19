/*
 * _Signal.cpp
 *
 *  Created on: 2013-3-18
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/signal/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SignalWrap : public SignalBase, public wrapper<SignalBase> {
public:
    SignalWrap(): SignalBase() {}
    SignalWrap(const string& name) : SignalBase(name) {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            SignalBase::_reset();
        }
    }

    void default_reset() {
        this->SignalBase::_reset();
    }

    SignalPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }
};


string (SignalBase::*sg_get_name)() const = &SignalBase::name;
void (SignalBase::*sg_set_name)(const string&) = &SignalBase::name;

void export_Signal() {
    class_<SignalWrap, boost::noncopyable>("SignalBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", sg_get_name, sg_set_name)
            //因为Indicator无法使用params['name']的形式，所以统一使用setParm/getParam
            //.add_property("params",
            //        make_function(&SignalBase::getParameter,
            //                return_internal_reference<>()))
            .def("getParam", &SignalBase::getParam<boost::any>)
            .def("setParam", &SignalBase::setParam<object>)
            .def("setTO", &SignalBase::setTO)
            .def("getTO", &SignalBase::getTO)
            .def("shouldBuy", &SignalBase::shouldBuy)
            .def("shouldSell", &SignalBase::shouldSell)
            .def("getBuySignal", &SignalBase::getBuySignal)
            .def("getSellSignal", &SignalBase::getSellSignal)
            .def("_addBuySignal", &SignalBase::_addBuySignal)
            .def("_addSellSignal", &SignalBase::_addSellSignal)
            .def("reset", &SignalBase::reset)
            .def("clone", &SignalBase::clone)
            .def("_calculate", pure_virtual(&SignalBase::_calculate))
            .def("_reset", &SignalBase::_reset, &SignalWrap::default_reset)
            .def("_clone", pure_virtual(&SignalBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<SignalBase>())
#endif
            ;

    register_ptr_to_python<SignalPtr>();

    def("SG_Bool", SG_Bool, (arg("buy"), arg("sell"), arg("kpart")="CLOSE"));
    def("SG_Single", SG_Single, (arg("ind"), arg("filter_n")=10,
            arg("filter_p")=0.1, arg("kpart")="CLOSE"));
    def("SG_Single2", SG_Single2, (arg("ind"), arg("filter_n")=10,
            arg("filter_p")=0.1, arg("kpart")="CLOSE"));
    def("SG_Cross", SG_Cross, (arg("fast"), arg("slow"), arg("kpart") = "CLOSE"));
    def("SG_CrossGold", SG_CrossGold, (arg("fast"), arg("slow"), arg("kpart") = "CLOSE"));
    def("SG_Flex", SG_Flex, (arg("op"), arg("slow_n"), arg("kpart") = "CLOSE"));
}


