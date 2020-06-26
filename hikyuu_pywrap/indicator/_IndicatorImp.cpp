/*
 * _IndicatorImp.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Indicator.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class IndicatorImpWrap : public IndicatorImp, public wrapper<IndicatorImp> {
public:
    IndicatorImpWrap() : IndicatorImp() {}
    IndicatorImpWrap(const string& name) : IndicatorImp(name) {}
    IndicatorImpWrap(const string& name, size_t result_num) : IndicatorImp(name, result_num) {}

    void _calculate(const Indicator& ind) {
        if (override call = get_override("_calculate")) {
            call(ind);
        } else {
            IndicatorImp::_calculate(ind);
        }
    }

    void default_calculate(const Indicator& ind) {
        this->IndicatorImp::_calculate(ind);
    }

    bool check() {
        if (override call = get_override("check")) {
            return call();
        } else {
            return IndicatorImp::check();
        }
    }

    bool default_check() {
        return this->IndicatorImp::check();
    }

    IndicatorImpPtr _clone() {
        if (override call = get_override("_clone")) {
            return call();
        } else {
            return IndicatorImp::_clone();
        }
    }

    IndicatorImpPtr default_clone() {
        return this->IndicatorImp::_clone();
    }

    bool isNeedContext() const {
        if (override call = get_override("isNeedContext")) {
            return call();
        } else {
            return IndicatorImp::isNeedContext();
        }
    }

    bool default_isNeedContext() const {
        return this->IndicatorImp::isNeedContext();
    }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(_set_overloads, _set, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(_append_overloads, _append, 1, 2)

string (IndicatorImp::*read_name)() const = &IndicatorImp::name;
void (IndicatorImp::*write_name)(const string&) = &IndicatorImp::name;

void export_IndicatorImp() {
    class_<IndicatorImpWrap, boost::noncopyable>("IndicatorImp", init<>())
      .def(init<const string&>())
      .def(init<const string&, size_t>())
      .def(self_ns::str(self))
      .add_property("name", read_name, write_name)
      .add_property("discard", &IndicatorImp::discard)
      .def("getParameter", &IndicatorImp::getParameter, return_value_policy<copy_const_reference>())
      .def("getParam", &IndicatorImp::getParam<boost::any>)
      .def("setParam", &IndicatorImp::setParam<object>)
      .def("setDiscard", &IndicatorImp::setDiscard)
      .def("_set", &IndicatorImp::_set, _set_overloads())
      .def("_readyBuffer", &IndicatorImp::_readyBuffer)
      .def("getResultNumber", &IndicatorImp::getResultNumber)
      .def("getResultAsPriceList", &IndicatorImp::getResultAsPriceList)
      .def("calculate", &IndicatorImp::calculate)
      .def("check", &IndicatorImp::check, &IndicatorImpWrap::default_check)
      .def("clone", &IndicatorImp::clone)
      .def("_calculate", &IndicatorImp::_calculate, &IndicatorImpWrap::default_calculate)
      .def("_clone", &IndicatorImp::_clone, &IndicatorImpWrap::default_clone)
      .def("isNeedContext", &IndicatorImp::isNeedContext, &IndicatorImpWrap::default_isNeedContext);

    register_ptr_to_python<IndicatorImpPtr>();
}
