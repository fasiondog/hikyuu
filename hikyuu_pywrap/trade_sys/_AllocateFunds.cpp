/*
 * _AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The portfolio-level fund allocation (AF) binding. AF carries three replaceable algorithm parts
 *  L1/L2/L3:
 *   - L1 _allocate   sub-system context -> weight
 *   - L2 _to_targets weight -> the executable quantity of the parent account
 *   - L3 _check_risk portfolio risk control clipping
 *  MM is restricted to the single system form, it no longer carries the portfolio-level allocation.
 *  See docs/design/pf_af_compat/design.md §5
 *  Created on: 2016-03-28
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyAllocateFundsBase : public AllocateFundsBase {
    PY_CLONE(PyAllocateFundsBase, AllocateFundsBase)

public:
    PyAllocateFundsBase() : AllocateFundsBase() {
        m_is_python_object = true;
    }

    PyAllocateFundsBase(const string& name) : AllocateFundsBase(name) {
        m_is_python_object = true;
    }

    PyAllocateFundsBase(const AllocateFundsBase& base) : AllocateFundsBase(base) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, AllocateFundsBase, _reset, );
    }

    // L1: sub-system context -> weight
    Weights _allocate(const Datetime& date, const TradeManagerPtr& tm, SubSystemContextList& contexts,
                      const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(Weights, AllocateFundsBase, "_allocate", _allocate, date, tm, contexts,
                               query);
    }

    // L2: weight -> the executable quantity of the parent account
    void _toTargets(const Datetime& date, const TradeManagerPtr& tm,
                    TradeSuggestionList& suggestions, const Weights& sys_weight,
                    const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(void, AllocateFundsBase, "_to_targets", _toTargets, date, tm,
                               suggestions, sys_weight, query);
    }

    // L3: portfolio risk control clipping
    void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                    TradeSuggestionList& suggestions, const KQuery& query) override {
        PYBIND11_OVERLOAD_NAME(void, AllocateFundsBase, "_check_risk", _checkRisk, date, tm,
                               suggestions, query);
    }
};

void export_AllocateFunds(py::module& m) {
    py::class_<AllocateFundsBase, AllocateFundsPtr, PyAllocateFundsBase>(
      m, "AllocateFundsBase", py::dynamic_attr(),
      R"(The portfolio-level fund allocation (AF) base class, used by the aggregate system (MultiSystem) only

AF is composed of three replaceable algorithm parts (each of them corresponds to one overload interface):

    - _allocate   [Required] L1 system-level allocation: sub-system context -> weight
    - _to_targets [Optional] L2 behavior-level conversion: weight -> the executable quantity of the parent account
    - _check_risk [Optional] L3 portfolio risk control clipping

Common parameters:

    - max-single-position=1.0 (float) : the L3 single instrument concentration upper limit (the proportion of the total assets); <=0 or >=1 means no limit
    - weight-list="" (str) : the L1 fixed weight list (comma separated); when it is not empty it overrides the equal weight default in order
    - fixed-amount=0.0 (float) : the L2 fixed amount; in mode A it is the target market value of every instrument, in mode B it is the quota of every sub-system)")
      .def(py::init<>())
      .def(py::init<const AllocateFundsBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<AllocateFundsBase>)
      .def("__repr__", to_py_str<AllocateFundsBase>)

      .def_property("name", py::overload_cast<>(&AllocateFundsBase::name, py::const_),
                    py::overload_cast<const string&>(&AllocateFundsBase::name),
                    py::return_value_policy::copy, "The algorithm part name")
      .def_property("tm", &AllocateFundsBase::getTM, &AllocateFundsBase::setTM,
                    "Set or get the trade management object")
      .def_property("query", &AllocateFundsBase::getQuery, &AllocateFundsBase::setQuery,
                    py::return_value_policy::copy, "Set or get the query condition")
      .def_property("mode", &AllocateFundsBase::getMode, &AllocateFundsBase::setMode,
                    py::return_value_policy::copy,
                    "The allocation mode: A (signal aggregation) / B (fund allocation)")

      .def("get_param", &AllocateFundsBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (AllocateFundsBase::*)(const std::string&, const boost::any&)>(
             &AllocateFundsBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &AllocateFundsBase::haveParam, "Whether the specified parameter exists")
      .def("reset", &AllocateFundsBase::reset, "The reset operation")
      .def("clone", &AllocateFundsBase::clone, "The clone operation")

      .def("allocate", &AllocateFundsBase::allocate, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("contexts"), py::arg("query"),
           R"(allocate(self, date, tm, suggestions, contexts, query)

    The unified entry of L1/L2/L3 (usually called by MultiSystem internally))")

      .def("_allocate", &AllocateFundsBase::_allocate, py::arg("date"), py::arg("tm"),
           py::arg("contexts"), py::arg("query"),
           R"(_allocate(self, date, tm, contexts, query)

    [Overload interface] L1 system-level allocation: decide the weight of every sub-system by the sub-system context

    :return: the weight table { System: weight })")

      .def("_to_targets", &AllocateFundsBase::_toTargets, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("sys_weight"), py::arg("query"),
           R"(_to_targets(self, date, tm, suggestions, sys_weight, query)

    [Overload interface] L2 behavior-level conversion: convert the weights into the executable quantity of the parent account, rewrite suggestions in place)")

      .def("_check_risk", &AllocateFundsBase::_checkRisk, py::arg("date"), py::arg("tm"),
           py::arg("suggestions"), py::arg("query"),
           R"(_check_risk(self, date, tm, suggestions, query)

    [Overload interface] L3 portfolio risk control clipping, rewrite the quantity of suggestions in place)")

      .def("_reset", &AllocateFundsBase::_reset,
           R"([Overload interface] The subclass reset interface, reset the internal private variables)")

        DEF_PICKLE(AllocateFundsPtr);

    //--------------------------------------------------------------------------------------
    // The built-in AF algorithms
    m.def("AF_EqualWeight", AF_EqualWeight, R"(AF_EqualWeight()

    The equal weight asset allocation; allocate the selected assets with an equal ratio (L1 equal weight 1/N))");

    m.def("AF_FixedAmount", AF_FixedAmount, py::arg("amount") = 20000.0, R"(AF_FixedAmount(amount=20000.0)

    The fixed amount asset allocation (L1 equal weight + L2 fixed amount)

    :param float amount: the maximum trading amount)");

    m.def("AF_FixedWeight", AF_FixedWeight, py::arg("weight") = 0.1, R"(AF_FixedWeight(weight=0.1)

    The fixed proportion asset allocation (L1 returns the fixed proportion directly, without normalization)

    :param float weight: the specified asset proportion (0, 1])");

    m.def("AF_FixedWeightList", AF_FixedWeightList, py::arg("weights"),
          R"(AF_FixedWeightList(weights)

    The fixed proportion list asset allocation (L1 takes weights[i] in order, without normalization)

    :param list weights: the specified asset proportion list)");

    m.def("AF_MultiFactor", AF_MultiFactor, R"(AF_MultiFactor()

    Create a MultiFactor scoring weight asset allocation algorithm instance, i.e. directly using the scores returned by the SE as the weights.)");
}
