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

    void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
        if (override call = get_override("_dyn_run_one_step")) {
            call(ind);
        } else {
            IndicatorImp::_dyn_run_one_step(ind, curPos, step);
        }
    }

    void default_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
        this->IndicatorImp::_dyn_run_one_step(ind, curPos, step);
    }

    bool supportIndParam() const {
        if (override call = get_override("support_ind_param")) {
            return call();
        } else {
            return IndicatorImp::supportIndParam();
        }
    }

    bool default_supportIndParam() const {
        return this->IndicatorImp::supportIndParam();
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

void (IndicatorImp::*set_ind_param1)(const string&, const Indicator&) = &IndicatorImp::setIndParam;
void (IndicatorImp::*set_ind_param2)(const string&, const IndParam&) = &IndicatorImp::setIndParam;

void export_IndicatorImp() {
    class_<IndicatorImpWrap, boost::noncopyable>("IndicatorImp", "指标实现基类", init<>())
      .def(init<const string&>())
      .def(init<const string&, size_t>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", read_name, write_name, "指标名称")
      .add_property("discard", &IndicatorImp::discard, "结果中需抛弃的个数")

      .def("get_parameter", &IndicatorImp::getParameter,
           return_value_policy<copy_const_reference>(), "获取内部参数类对象")

      .def("have_param", &IndicatorImp::haveParam)
      .def("get_param", &IndicatorImp::getParam<boost::any>)
      .def("set_param", &IndicatorImp::setParam<object>)
      .def("have_ind_param", &IndicatorImp::haveIndParam)
      .def("support_ind_param", &IndicatorImp::supportIndParam,
           &IndicatorImpWrap::default_supportIndParam)
      .def("get_ind_param", &IndicatorImp::getIndParam)
      .def("set_ind_param", set_ind_param1)
      .def("set_ind_param", set_ind_param2)
      .def("set_discard", &IndicatorImp::setDiscard)
      .def("_set", &IndicatorImp::_set, _set_overloads())
      .def("_ready_buffer", &IndicatorImp::_readyBuffer)
      .def("get_result_num", &IndicatorImp::getResultNumber)
      .def("get_result_as_price_list", &IndicatorImp::getResultAsPriceList)
      .def("calculate", &IndicatorImp::calculate)
      .def("check", &IndicatorImp::check, &IndicatorImpWrap::default_check)
      .def("clone", &IndicatorImp::clone)
      .def("_calculate", &IndicatorImp::_calculate, &IndicatorImpWrap::default_calculate)
      .def("_dyn_run_one_step", &IndicatorImp::_dyn_run_one_step,
           &IndicatorImpWrap::default_dyn_run_one_step)
      .def("_clone", &IndicatorImp::_clone, &IndicatorImpWrap::default_clone)
      .def("is_need_context", &IndicatorImp::isNeedContext,
           &IndicatorImpWrap::default_isNeedContext)
      .def("is_leaf", &IndicatorImp::isLeaf);

    register_ptr_to_python<IndicatorImpPtr>();
}
