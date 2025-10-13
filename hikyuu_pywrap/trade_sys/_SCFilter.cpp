/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-04
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/multifactor/buildin_scfilter.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyScoresFilterBase : public ScoresFilterBase {
    PY_CLONE(PyScoresFilterBase, ScoresFilterBase)

public:
    using ScoresFilterBase::ScoresFilterBase;

    PyScoresFilterBase(const ScoresFilterBase& base) : ScoresFilterBase(base) {}

    virtual ScoreRecordList filter(const ScoreRecordList& scores, const Datetime& date,
                                   const KQuery& query) override {
        PYBIND11_OVERRIDE_PURE(ScoreRecordList, ScoresFilterBase, filter, scores, date, query);
    }
};

void export_SCFilter(py::module& m) {
    py::class_<ScoresFilterBase, ScoresFilterPtr, PyScoresFilterBase>(
      m, "ScoresFilterBase", py::dynamic_attr(), R"(用于 MF 的截面标准化操作)")
      .def(py::init<>())
      .def(py::init<const ScoresFilterBase&>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<ScoresFilterBase>)
      .def("__repr__", to_py_str<ScoresFilterBase>)

      .def_property("name", py::overload_cast<>(&ScoresFilterBase::name, py::const_),
                    py::overload_cast<const string&>(&ScoresFilterBase::name),
                    py::return_value_policy::copy, "名称")

      .def("get_param", &ScoresFilterBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &ScoresFilterBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &ScoresFilterBase::haveParam, "是否存在指定参数")

      .def("clone", &ScoresFilterBase::clone, "克隆操作")

      .def("filter", &ScoresFilterBase::filter, "【重载接口】子类计算接口")

        DEF_PICKLE(ScoresFilterPtr);
}