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
      m, "ScoresFilterBase", py::dynamic_attr(), R"(The cross-section standardization operation used for the MF)")
      .def(py::init<>())
      .def(py::init<const ScoresFilterBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<ScoresFilterBase>)
      .def("__repr__", to_py_str<ScoresFilterBase>)

      .def_property("name", py::overload_cast<>(&ScoresFilterBase::name, py::const_),
                    py::overload_cast<const string&>(&ScoresFilterBase::name),
                    py::return_value_policy::copy, "Name")

      .def("get_param", &ScoresFilterBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (ScoresFilterBase::*)(const std::string&, const boost::any&)>(
             &ScoresFilterBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ScoresFilterBase::haveParam, "Whether the specified parameter exists")

      .def("clone", &ScoresFilterBase::clone, "The clone operation")

      .def("filter", &ScoresFilterBase::filter, R"(filter(self, scores, date, query)
        
    The cross-section filter
    :param list scores: the cross-section data
    :param Datetime date: the cross-section date
    :param KQuery query: the query parameters
    :return: the cross-section data
    :rtype: ScoreRecordList)")

      .def("_filter", &ScoresFilterBase::_filter, "[Overload interface] The subclass calculation interface")

      .def("__or__",
           [](const ScoresFilterPtr& self, const ScoresFilterPtr& other) { return self | other; })

        DEF_PICKLE(ScoresFilterPtr);

    m.def("SCFilter_IgnoreNan", &SCFilter_IgnoreNan, R"(SCFilter_IgnoreNan() -> ScoresFilterPtr
      
    Ignore the NAN values in the cross-section)");

    m.def("SCFilter_LessOrEqualValue", &SCFilter_LessOrEqualValue, py::arg("value") = 0.0,
          R"(SCFilter_LessOrEqualValue([value = 0.0])
            
    Filter out the cross-sections whose score is less than or equal to the specified value)");

    m.def("SCFilter_TopN", &SCFilter_TopN, py::arg("topn") = 10,
          R"(SCFilter_TopN([topn: int=10])
            
    Get the first topn of the score list
    
    :param int topn: the first topn)");

    m.def("SCFilter_Group", &SCFilter_Group, py::arg("group") = 10, py::arg("group_index") = 0,
          R"(SCFilter_Group([group: int=10, group_index: int=0])
            
    Group and filter by the cross-section
    :param int group: the number of the groups
    :param int group_index: the group index
    :return: the cross-section filter
    :rtype: ScoresFilterPtr)");

    m.def("SCFilter_AmountLimit", &SCFilter_AmountLimit, py::arg("min_amount_percent_limit") = 0.1,
          R"(SCFilter_AmountLimit([min_amount_percent_limit: float = 0.1])
            
    Filter out the cross-sections whose amount is within the percentage range at the end of the score list
    
    Note: it is related to the order of the passed cross-section score list; if it is descending, the system score records with the smaller amounts are filtered; otherwise, the records with the larger amounts

    :param double min_amount_percent_limit: the minimum amount percentage limit
    :return: the cross-section filter
    :rtype: ScoresFilterPtr)");

    m.def("SCFilter_Price", &SCFilter_Price, py::arg("min_price") = 10.,
          py::arg("max_price") = 100000.,
          R"(SCFilter_Price([min_price = 10., max_price = 100000.])
            
    Keep only the targets whose prices are within [min_price, max_price]
    
    Note: it is related to the order of the passed cross-section score list; if it is descending, the system score records with the smaller prices are filtered; otherwise, the records with the larger prices

    :param double min_price: the minimum price limit
    :param double max_price: the maximum price limit)");
}