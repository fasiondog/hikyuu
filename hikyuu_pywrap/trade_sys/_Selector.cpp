/*
 * _Selector.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/selector/build_in.h>
#include "../_Parameter.h"
#include "../pybind_utils.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class SelectorWrap : public SelectorBase, public wrapper<SelectorBase> {
public:
    SelectorWrap() : SelectorBase() {}
    SelectorWrap(const string& name) : SelectorBase(name) {}
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
        return this->get_override("get_selected_system_list")(date);
    }

    SelectorPtr _clone() {
        return this->get_override("_clone")();
    }

    void _calculate() {
        this->get_override("_calculate")();
    }
};

string (SelectorBase::*sb_get_name)() const = &SelectorBase::name;
void (SelectorBase::*sb_set_name)(const string&) = &SelectorBase::name;

SelectorPtr SE_Fixed(py::list stock_list, const SystemPtr& sys) {
    StockList stk_list = python_list_to_vector<StockList>(stock_list);
    return SE_Fixed(stk_list, sys);
}

SelectorPtr (*SE_Fixed_1)() = SE_Fixed;
SelectorPtr (*SE_Fixed_2)(py::list stock_list, const SystemPtr& sys) = SE_Fixed;
// SelectorPtr (*SE_Fixed_2)(const StockList&, const SYSPtr&) = SE_Fixed;

void export_Selector() {
    class_<SelectorWrap, boost::noncopyable>("SelectorBase", init<>())
      .def(init<const string&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", sb_get_name, sb_set_name)
      .def("get_param", &SelectorBase::getParam<boost::any>)
      .def("set_param", &SelectorBase::setParam<object>)
      .def("have_param", &SelectorBase::haveParam)

      .def("reset", &SelectorBase::reset)
      .def("clone", &SelectorBase::clone)
      .def("_reset", &SelectorBase::_reset, &SelectorWrap::default_reset)
      .def("_clone", pure_virtual(&SelectorBase::_clone))
      .def("_calculate", pure_virtual(&SelectorBase::_calculate))
      .def("get_selected_system_list", pure_virtual(&SelectorBase::getSelectedSystemList))
      .def("add_stock", &SelectorBase::addStock)
      //.def("add_stock_list", &SelectorBase::addStockList)  // 在python中扩展
      .def("clear", &SelectorBase::clear)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<SelectorBase>())
#endif
      ;

    register_ptr_to_python<SelectorPtr>();

    def("SE_Fixed", SE_Fixed_1);
    def("SE_Fixed", SE_Fixed_2);
}
