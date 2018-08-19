/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/selector/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SelectorWrap : public SelectorBase, public wrapper<SelectorBase> {
public:
    SelectorWrap(): SelectorBase() {}
    SelectorWrap(const string& name): SelectorBase(name) {}
    virtual ~SelectorWrap() {}

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            SelectorBase::_reset();
        }
    }

    void default_reset() {
        this->SelectorBase::_reset();
    }

    SystemList getSelectedSystemList(Datetime date) {
        return this->get_override("getSelectedSystemList")(date);
    }

    SelectorPtr _clone() {
        return this->get_override("_clone")();
    }
};

string (SelectorBase::*sb_get_name)() const = &SelectorBase::name;
void (SelectorBase::*sb_set_name)(const string&) = &SelectorBase::name;

SelectorPtr (*SE_Fixed_1)() = SE_Fixed;
SelectorPtr (*SE_Fixed_2)(const StockList&, const SYSPtr&) = SE_Fixed;

void export_Selector() {
    class_<SelectorWrap, boost::noncopyable>("SelectorBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", sb_get_name, sb_set_name)
            .def("getParam", &SelectorBase::getParam<boost::any>)
            .def("setParam", &SelectorBase::setParam<object>)

            .def("reset", &SelectorBase::reset)
            .def("clone", &SelectorBase::clone)
            .def("_reset", &SelectorBase::_reset, &SelectorWrap::default_reset)
            .def("_clone", pure_virtual(&SelectorBase::_clone))
            .def("getSelectedSystemList", pure_virtual(&SelectorBase::getSelectedSystemList))
            .def("addStock", &SelectorBase::addStock)
            .def("addStockList", &SelectorBase::addStockList)
            .def("clear", &SelectorBase::clear)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<SelectorBase>())
#endif
            ;

    register_ptr_to_python<SelectorPtr>();

    def("SE_Fixed", SE_Fixed_1);
    def("SE_Fixed", SE_Fixed_2);

}


