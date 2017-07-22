/*
 * _AllocateMoney.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class AllocateMoneyWrap : public AllocateMoneyBase, public wrapper<AllocateMoneyBase> {
public:
    AllocateMoneyWrap(): AllocateMoneyBase() {}
    AllocateMoneyWrap(const TradeManagerPtr& tm) : AllocateMoneyBase(tm) {}
    virtual ~AllocateMoneyWrap() {}

    string name() const {
        if (override name = this->get_override("name"))
#if defined(BOOST_WINDOWS)
            return call<char const*>(name.ptr());
#else
            return name();
#endif
        return AllocateMoneyBase::name();
    }

    string default_name() const {
        return this->AllocateMoneyBase::name();
    }

    void _reset() {
        if (override func = this->get_override("_reset")) {
            func();
        } else {
            AllocateMoneyBase::_reset();
        }
    }

    void default_reset() {
        this->AllocateMoneyBase::_reset();
    }

    StockList tryAllocate(Datetime date, const StockList& stocks) {
        if (override func = this->get_override("tryAllocate")) {
            return func(date, stocks);
        } else {
            return AllocateMoneyBase::tryAllocate(date, stocks);
        }
    }

    StockList default_tryAllocate(Datetime date, const StockList& stocks) {
        return this->AllocateMoneyBase::tryAllocate(date, stocks);
    }

    AllocateMoneyPtr _clone() {
        return this->get_override("_clone")();
    }
};

void export_AllocateMoney() {
    class_<AllocateMoneyWrap, boost::noncopyable>("AllocateMoneyBase", init<>())
            .def(init<const TradeManagerPtr&>())
            .def(self_ns::str(self))
            .add_property("params",
                    make_function(&AllocateMoneyBase::getParameter,
                            return_internal_reference<>()))
            .add_property("tm", &AllocateMoneyBase::getTM, &AllocateMoneyBase::setTM)
            .def("name", &AllocateMoneyBase::name, &AllocateMoneyWrap::default_name)
            .def("reset", &AllocateMoneyBase::reset)
            .def("clone", &AllocateMoneyBase::clone)
            .def("_reset", &AllocateMoneyBase::_reset, &AllocateMoneyWrap::default_reset)
            .def("_clone", pure_virtual(&AllocateMoneyBase::_clone))
            .def("tryAllocate", &AllocateMoneyBase::tryAllocate, &AllocateMoneyWrap::default_tryAllocate)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<AllocateMoneyBase>())
#endif
            ;

    register_ptr_to_python<AllocateMoneyPtr>();
}


