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
    virtual ~SelectorWrap() {}

    string name() const {
        if (override name = this->get_override("name"))
#if defined(BOOST_WINDOWS)
            return call<char const*>(name.ptr());
#else
            return name();
#endif
        return SelectorBase::name();
    }

    string default_name() const {
        return this->SelectorBase::name();
    }

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

    StockList getSelectedStock(Datetime date) {
        if (override func = this->get_override("getSelectedStock")) {
            return func(date);
        } else {
            return SelectorBase::getSelectedStock(date);
        }
    }

    StockList default_getSelectedStock(Datetime date) {
        return this->SelectorBase::getSelectedStock(date);
    }

    SelectorPtr _clone() {
        return this->get_override("_clone")();
    }
};

void export_Selector() {
    class_<SelectorWrap, boost::noncopyable>("SelectorBase", init<>())
            .def(self_ns::str(self))
            .add_property("params",
                    make_function(&SelectorBase::getParameter,
                            return_internal_reference<>()))
            .def("name", &SelectorBase::name, &SelectorWrap::default_name)
            .def("reset", &SelectorBase::reset)
            .def("clone", &SelectorBase::clone)
            .def("_reset", &SelectorBase::_reset, &SelectorWrap::default_reset)
            .def("_clone", pure_virtual(&SelectorBase::_clone))
            .def("addStock", &SelectorBase::addStock)
            .def("addStockList", &SelectorBase::addStockList)
            .def("getRawStockList", &SelectorBase::getRawStockList, &SelectorWrap::getRawStockList)
            .def("clearStockList", &SelectorBase::clearStockList)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<SelectorBase>())
#endif
            ;

    register_ptr_to_python<SelectorPtr>();
}


