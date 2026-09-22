/*
 * _Selector.cpp
 *
 *  Created on: 2016-03-28
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

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

    SystemWeightList _allocateWeight(const Datetime& date,
                                     const SystemWeightList& se_list) override {
        PYBIND11_OVERLOAD_PURE_NAME(SystemWeightList, AllocateFundsBase, "_allocate_weight",
                                    _allocateWeight, date, se_list);
    }
};

void export_AllocateFunds(py::module& m) {
    py::class_<AllocateFundsBase, AFPtr, PyAllocateFundsBase>(m, "AllocateFundsBase",
                                                              py::dynamic_attr(),
                                                              R"(The asset allocation algorithm base class; the subclass interfaces:

Common parameters:
    
    - adjust_running_sys (bool|True): whether to adjust the positions of the strategies already holding positions. When not adjusting, only the current remaining funds of the total account are used for the allocation; otherwise the total market value is used.
        Note: regardless of whether the holding strategies are adjusted, the weight ratios are relative to the total assets, not the remaining cash balance.
              Adjusting only by the remaining cash ratio is meaningless; even if allocated, the actual trade may not be completed due to the trading costs.
        adjust_running_sys: True - actively increase or decrease the positions of the holding strategies according to the asset allocation
        adjust_running_sys: False - the holding strategies will not be forcibly increased or decreased according to the current allocated weights
    
    - auto_adjust_weight (bool|True): adjust the weights automatically; in this case the passed weights are considered to be the mutual ratios of the securities (see the ignore_zero_weight description). Otherwise, the passed weights are used as the specified weights without adjustment (in this case each passed weight needs to be less than 1).
  
    - ignore_zero_weight (bool|False): this parameter takes effect when auto_adjust_weight is True. Whether to filter the 0 values (including those less than 0) and the nan values in the ratio weight list returned by the subclass.

        E.g.: if the subclass returns the weight ratio list [6, 2, 0, 0, 0], then
            - filtering the 0 values, the actually adjusted weight is Xi / sum(Xi): [6/8, 2/8]
            - not filtering, let m be the number of the non-zero elements and n the total number of elements, (Xi / Sum(Xi)) * (m / n):
                 [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0]
                i.e. after keeping it divided into 5 shares, the relative ratio is kept only in 2 shares

    - ignore_se_score_is_null (bool|False): Ignore the systems whose score is null in the selected system list. Note: some SEs (e.g. SE_MultiFactor) may also have a similar control themselves
    - ignore_se_score_lt_zero (bool|False): Ignore the systems whose score is less than or equal to 0 in the selected system list
    - reserve_percent (float|0.0): the reserved asset proportion that does not participate in the reallocation
    - trace (bool|False): print the tracking information

The subclass interfaces:

    - _allocateWeight : [Required] The subclass asset allocation adjustment implementation
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const AllocateFundsBase&>())
      .def("__str__", to_py_str<AllocateFundsBase>)
      .def("__repr__", to_py_str<AllocateFundsBase>)
      .def_property("name", py::overload_cast<>(&AllocateFundsBase::name, py::const_),
                    py::overload_cast<const string&>(&AllocateFundsBase::name),
                    py::return_value_policy::copy, "The algorithm component name")
      .def_property("query", py::overload_cast<>(&AllocateFundsBase::getQuery, py::const_),
                    py::overload_cast<const KQuery&>(&AllocateFundsBase::setQuery),
                    py::return_value_policy::copy, "Set or get the query condition")
      .def_property_readonly("tm", py::overload_cast<>(&AllocateFundsBase::getTM, py::const_),
                             py::return_value_policy::copy)

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
      .def("_reset", &AllocateFundsBase::_reset, "The subclass reset operation implementation")
      .def("_allocate_weight", &AllocateFundsBase::_allocateWeight, py::arg("date"),
           py::arg("se_list"),
           R"(_allocate_weight(self, date, se_list)

        [Overload interface] The subclass weight allocation interface, getting the actually allocated system instances and their weights

        :param Datetime date: the current time
        :param SystemList se_list: the list of the currently selected systems
        :return: the list of the system weight allocation information
        :rtype: SystemWeightList)")

        DEF_PICKLE(AFPtr);

    m.def("AF_EqualWeight", AF_EqualWeight, R"(AF_EqualWeight()
    
    The equal weight asset allocation; allocate the selected assets with an equal ratio)");

    m.def("AF_FixedAmount", AF_FixedAmount, py::arg("amount") = 20000.0, R"(AF_FixedAmount(amount)
    
    The equal amount asset allocation; allocate the selected assets with an equal amount

    :param float amount: the maximum trading amount)");

    m.def("AF_FixedWeight", AF_FixedWeight, py::arg("weight") = 0.1, R"(AF_FixedWeight(weight)
    
    The fixed-ratio asset allocation

    :param float weight:  the specified asset proportion [0, 1])");

    m.def("AF_FixedWeightList", AF_FixedWeightList, py::arg("weights"),
          R"(AF_FixedWeightList(weights)
    
    The fixed-ratio asset allocation list.

    :param float weights:  the specified asset proportion list)");

    m.def("AF_MultiFactor", AF_MultiFactor, R"(AF_MultiFactor()
      
    Create a MultiFactor scoring weight asset allocation algorithm instance, i.e. directly using the scores returned by the SE as the weights.)");
}