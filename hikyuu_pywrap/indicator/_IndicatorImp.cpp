/*
 * _IndicatorImp.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyIndicatorImp : public IndicatorImp {
    PY_CLONE(PyIndicatorImp, IndicatorImp)

public:
    using IndicatorImp::IndicatorImp;

    bool check() override {
        PYBIND11_OVERLOAD(bool, IndicatorImp, check, );
    }

    void _calculate(const Indicator& ind) override {
        PYBIND11_OVERLOAD(void, IndicatorImp, _calculate, ind);
    }

    bool isNeedContext() const override {
        PYBIND11_OVERLOAD_NAME(bool, IndicatorImp, "is_need_context", isNeedContext, );
    }

    void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) override {
        PYBIND11_OVERLOAD(void, IndicatorImp, _dyn_run_one_step, ind, curPos, step);
    }

    bool supportIndParam() const override {
        PYBIND11_OVERLOAD_NAME(bool, IndicatorImp, "support_ind_param", supportIndParam, );
    }

    bool isSerial() const override {
        PYBIND11_OVERLOAD_NAME(bool, IndicatorImp, "is_serial", isSerial, );
    }

    void _dyn_calculate(const Indicator& ind) override {
        PYBIND11_OVERLOAD(void, IndicatorImp, _dyn_calculate, ind);
    }
};

const string& (IndicatorImp::*read_name)() const = &IndicatorImp::name;
void (IndicatorImp::*write_name)(const string&) = &IndicatorImp::name;

void (IndicatorImp::*set_ind_param1)(const string&, const Indicator&) = &IndicatorImp::setIndParam;
void (IndicatorImp::*set_ind_param2)(const string&, const IndParam&) = &IndicatorImp::setIndParam;

void export_IndicatorImp(py::module& m) {
    py::class_<IndicatorImp, IndicatorImpPtr, PyIndicatorImp>(
      m, "IndicatorImp", R"(指标实现类，定义新指标时，应从此类继承
    
    子类需实现以下接口：

        - _clone() -> IndicatorImp
        - _calculate(ind) ：指标计算
        - isNeedContext(bool) ：是否依赖上下文)")
      .def(py::init<>())

      .def(py::init<const string&>(), R"(
    :param str name: 指标名称)")

      .def(py::init<const string&, size_t>(), R"(
    :param str name: 指标名称
    :param int result_num: 指标结果集数量)")

      .def("__str__", to_py_str<IndicatorImp>)
      .def("__repr__", to_py_str<IndicatorImp>)

      .def_property("name", read_name, write_name, py::return_value_policy::copy, "指标名称")
      .def_property_readonly("discard", &IndicatorImp::discard, "结果中需抛弃的个数")

      .def("get_parameter", &IndicatorImp::getParameter, py::return_value_policy::copy,
           "获取内部参数类对象")

      .def("have_param", &IndicatorImp::haveParam)
      .def("get_param", &IndicatorImp::getParam<boost::any>)
      .def("set_param", &IndicatorImp::setParam<boost::any>)
      .def("have_ind_param", &IndicatorImp::haveIndParam)
      .def("support_ind_param", &IndicatorImp::supportIndParam)
      .def("get_ind_param", &IndicatorImp::getIndParam)
      .def("set_ind_param", set_ind_param1)
      .def("set_ind_param", set_ind_param2)
      .def("set_discard", &IndicatorImp::setDiscard)
      .def("_set", &IndicatorImp::_set, py::arg("val"), py::arg("pos"), py::arg("num") = 0)
      .def("_ready_buffer", &IndicatorImp::_readyBuffer)
      .def("get_result_num", &IndicatorImp::getResultNumber)
      .def("get_result_as_price_list", &IndicatorImp::getResultAsPriceList)
      .def("calculate", &IndicatorImp::calculate)
      .def("check", &IndicatorImp::check)
      .def("clone", &IndicatorImp::clone)
      .def("_calculate", &IndicatorImp::_calculate)
      .def("_dyn_run_one_step", &IndicatorImp::_dyn_run_one_step)
      .def("_dyn_calculate", &IndicatorImp::_dyn_calculate)
      .def("is_need_context", &IndicatorImp::isNeedContext)
      .def("is_leaf", &IndicatorImp::isLeaf)
      .def("is_serial", &IndicatorImp::isSerial)

        DEF_PICKLE(IndicatorImpPtr);
}
