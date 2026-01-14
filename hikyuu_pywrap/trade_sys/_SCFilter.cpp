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
    PyScoresFilterBase() : ScoresFilterBase() {
        m_is_python_object = true;
    }

    PyScoresFilterBase(const string& name) : ScoresFilterBase(name) {
        m_is_python_object = true;
    }

    PyScoresFilterBase(const ScoresFilterBase& base) : ScoresFilterBase(base) {
        m_is_python_object = true;
    }

    virtual ScoreRecordList _filter(const ScoreRecordList& scores, const Datetime& date,
                                    const KQuery& query) override {
        PYBIND11_OVERRIDE_PURE(ScoreRecordList, ScoresFilterBase, _filter, scores, date, query);
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

      .def("filter", &ScoresFilterBase::filter, R"(filter(self, scores, date, query)
        
    截面过滤
    :param list scores: 截面数据
    :param Datetime date: 截面日期
    :param KQuery query: 查询参数
    :return: 截面数据
    :rtype: ScoreRecordList)")

      .def("_filter", &ScoresFilterBase::_filter, "【重载接口】子类计算接口")

      .def("__or__",
           [](const ScoresFilterPtr& self, const ScoresFilterPtr& other) { return self | other; })

        DEF_PICKLE(ScoresFilterPtr);

    m.def("SCFilter_IgnoreNan", &SCFilter_IgnoreNan, R"(SCFilter_IgnoreNan() -> ScoresFilterPtr
      
    忽略截面中的NAN值)");

    m.def("SCFilter_LessOrEqualValue", &SCFilter_LessOrEqualValue, py::arg("value") = 0.0,
          R"(SCFilter_LessOrEqualValue([value = 0.0])
            
    过滤掉评分小于等于指定值的截面)");

    m.def("SCFilter_TopN", &SCFilter_TopN, py::arg("topn") = 10,
          R"(SCFilter_TopN([topn: int=10])
            
    获取评分列表中的前 topn 个
    
    :param int topn: 前 topn 个)");

    m.def("SCFilter_Group", &SCFilter_Group, py::arg("group") = 10, py::arg("group_index") = 0,
          R"(SCFilter_Group([group: int=10, group_index: int=0])
            
    按截面进行分组过滤
    :param int group: 分组数量
    :param int group_index: 分组索引
    :return: 截面过滤器
    :rtype: ScoresFilterPtr)");

    m.def("SCFilter_AmountLimit", &SCFilter_AmountLimit, py::arg("min_amount_percent_limit") = 0.1,
          R"(SCFilter_AmountLimit([min_amount_percent_limit: float = 0.1])
            
    过滤掉成交金额在评分列表末尾百分比范围内的截面
    
    注意：和传入的截面评分列表顺序相关，如果是降序，过滤的是成交金额较小的系统评分记录；反之，则是金额较大的系统评分记录

    :param double min_amount_percent_limit: 最小金额百分比限制
    :return: 截面过滤器
    :rtype: ScoresFilterPtr)");

    m.def("SCFilter_Price", &SCFilter_Price, py::arg("min_price") = 10.,
          py::arg("max_price") = 100000.,
          R"(SCFilter_Price([min_price = 10., max_price = 100000.])
            
    仅保留价格在 [min_price, max_price] 之间的标的
    
    注意：和传入的截面评分列表顺序相关，如果是降序，过滤的是价格较小的系统评分记录；反之，则是价格较大的系统评分记录

    :param double min_price: 最小价格限制
    :param double max_price: 最大价格限制)");
}