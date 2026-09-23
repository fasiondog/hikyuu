/*
 * _Selector.cpp
 *
 *  Created on: 2016-03-28
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/selector/build_in.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySelectorBase : public SelectorBase {
    PY_CLONE(PySelectorBase, SelectorBase)

public:
    PySelectorBase() : SelectorBase() {
        m_is_python_object = true;
    }

    PySelectorBase(const string& name) : SelectorBase(name) {
        m_is_python_object = true;
    }

    PySelectorBase(const SelectorBase& base) : SelectorBase(base) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SelectorBase, _reset, );
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, SelectorBase, _calculate, );
    }

    void _addSystem(const SystemPtr& sys) override {
        PYBIND11_OVERLOAD(void, SelectorBase, _addSystem, sys);
    }

    void _removeAll() override {
        PYBIND11_OVERLOAD(void, SelectorBase, _removeAll, );
    }

    // The subclass interfaces that must be implemented
    SystemWeightList _getSelected(Datetime date) override {
        py::gil_scoped_acquire gil;
        py::function py_func =
          py::get_override(static_cast<const SelectorBase*>(this), "get_selected");
        if (!py_func) {
            pybind11::pybind11_fail(
              "Tried to call pure virtual function \"SelectorBase::get_selected\"");
        }

        py::sequence py_ret = py_func(date);
        auto c_ret = python_list_to_vector<SystemWeight>(py_ret);
        return c_ret;
    }



    string str() const override {
        PYBIND11_OVERRIDE_NAME(string, SelectorBase, "__str__", str, );
    }
};

#ifdef __GNUC__
#pragma GCC visibility push(hidden)
#endif
class PyOptimalSelector : public OptimalSelectorBase {
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    PyOptimalSelector() : OptimalSelectorBase("SE_PyOptimal") {
        m_is_python_object = false;
    }

    explicit PyOptimalSelector(const py::function& evalfunc)
    : OptimalSelectorBase("SE_PyOptimal"), m_evaluate(evalfunc) {
        m_is_python_object = false;
    }

    virtual ~PyOptimalSelector() = default;

public:
    virtual SelectorPtr _clone() override {
        return std::make_shared<PyOptimalSelector>();
    }

    virtual double evaluate(const SYSPtr& sys, const Datetime& lastDate) noexcept override {
        double ret = Null<double>();
        try {
            ret = m_evaluate(sys, lastDate).cast<double>();
        } catch (const std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknown error!");
        }
        return ret;
    }

private:
    // Cannot be serialized currently
    py::function m_evaluate;
};
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

SEPtr crtSEOptimal(const py::function& evalfunc) {
    return std::make_shared<PyOptimalSelector>(evalfunc);
}

void export_Selector(py::module& m) {
    py::class_<SystemWeight>(m, "SystemWeight", py::dynamic_attr(),
                             "The system weight structure; during the asset allocation, it specifies the asset proportion coefficient of the corresponding system")
      .def(py::init<>())
      .def(py::init<const SystemPtr&, price_t>())
      .def("__str__", to_py_str<SystemWeight>)
      .def("__repr__", to_py_str<SystemWeight>)
      .def_readwrite("sys", &SystemWeight::sys, "The corresponding System instance")
      .def_readwrite("weight", &SystemWeight::weight)

        DEF_PICKLE(SystemWeight);

    m.def("systemweights_to_np", [](const SystemWeightList& swl) {
        size_t total = swl.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
            int32_t sys_name[20];
            int32_t code[10];
            int32_t name[20];
            double weight;
        };

        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, total = swl.size(); i < total; i++) {
            const SystemWeight& sw = swl[i];
            if (sw.sys) {
                utf8_to_utf32(sw.sys->name(), data[i].sys_name, 20);
                utf8_to_utf32(sw.sys->getStock().market_code(), data[i].code, 10);
                utf8_to_utf32(sw.sys->getStock().name(), data[i].name, 20);
            } else {
                memset(data[i].sys_name, 0, 20 * sizeof(int32_t));
                memset(data[i].code, 0, 10 * sizeof(int32_t));
                memset(data[i].name, 0, 20 * sizeof(int32_t));
            }
            data[i].weight = sw.weight;
        }
        // Define the NumPy structured data type
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>(
                      {htr("sys_name"), htr("market_code"), htr("stock_name"), htr("weight")}),
                    vector_to_python_list<string>({"U20", "U10", "U20", "d"}),
                    vector_to_python_list<int64_t>({0, 80, 120, 200}), 208);

        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("systemweights_to_df", [](const SystemWeightList& sws) {
        size_t total = sws.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the python string object array
        py::list sysname_list(total);
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<double> value_arr(total);

        // Get the buffer of the value array
        auto value_buf = value_arr.request();
        double* value_ptr = static_cast<double*>(value_buf.ptr);

        // Fill the data
        for (size_t i = 0; i < total; i++) {
            const SystemWeight& sw = sws[i];
            if (sw.sys) {
                sysname_list[i] = py::str(sw.sys->name());
                code_list[i] = py::str(sw.sys->getStock().market_code());
                name_list[i] = py::str(sw.sys->getStock().name());
                value_ptr[i] = sw.weight;
            } else {
                sysname_list[i] = py::str("");
                code_list[i] = py::str("");
                name_list[i] = py::str("");
                value_ptr[i] = Null<double>();
            }
        }

        // Build the DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("sys_name").c_str()] =
          pandas.attr("Series")(sysname_list, py::arg("dtype") = "string");
        columns[htr("market_code").c_str()] =
          pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("stock_name").c_str()] =
          pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns["weight"] = value_arr;

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
    });

    py::class_<SelectorBase, SEPtr, PySelectorBase>(
      m, "SelectorBase",
      R"(The selector strategy base class, implementing the algorithm for evaluating and selecting the targets and the system strategies; the custom selector strategy subclass interfaces:

    - get_selected - [Required] Get the list of the system instances selected at the specified moment
    - _calculate - [Required] The calculation interface
    - _reset - [Optional] Reset the private attributes
    - _clone - [Required] The clone interface)")

      .def(py::init<>())
      .def(py::init<const SelectorBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<SelectorBase>)
      .def("__repr__", to_py_str<SelectorBase>)

      .def_property("name", py::overload_cast<>(&SelectorBase::name, py::const_),
                    py::overload_cast<const string&>(&SelectorBase::name),
                    py::return_value_policy::copy, "The algorithm name")
      .def_property_readonly("proto_sys_list", &SelectorBase::getProtoSystemList,
                             py::return_value_policy::copy, "The prototype system list")
      .def_property_readonly("real_sys_list", &SelectorBase::getRealSystemList,
                             py::return_value_policy::copy, "The actual running system list set by the PF at runtime")
      .def_property_readonly("scfilter", &SelectorBase::getScoresFilter, "Get the ScoresFilter")

      .def_property(
        "mf", &SelectorBase::getMF,
        [](SelectorBase& self, py::object mf) {
            py::gil_scoped_acquire gil;
            auto tmp = mf;
            self.setMF(mf.cast<MFPtr>());
            tmp.release();
        },
        "Get the associated MF")

      .def("get_param", &SelectorBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (SelectorBase::*)(const std::string&, const boost::any&)>(
             &SelectorBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &SelectorBase::haveParam, "Whether the specified parameter exists")

      .def("reset", &SelectorBase::reset, "The reset operation")
      .def("clone", &SelectorBase::clone, "The clone operation")
      .def("remove_all", &SelectorBase::removeAll, "Remove all the added prototype systems")

      .def("add_stock", &SelectorBase::addStock, py::arg("stock"), py::arg("sys"),
           R"(add_stock(self, stock, sys)

    Add the initial target and its corresponding system strategy prototype

    :param Stock stock: the initial target to add
    :param System sys: the system strategy prototype)")

      .def(
        "add_stock_list",
        [](SelectorBase& self, py::object stk_list, const SYSPtr& sys) {
            self.addStockList(get_stock_list_from_python(stk_list), sys);
        },
        py::arg("stk_list"), py::arg("sys"),
        R"(add_stock_list(self, stk_list, sys)

    Add the initial target list and its system strategy prototype

    :param StockList stk_list: the initial target list to add
    :param System sys: the system strategy prototype)")

      .def("get_proto_sys_list", &SelectorBase::getProtoSystemList, py::return_value_policy::copy)
      .def("get_real_sys_list", &SelectorBase::getRealSystemList, py::return_value_policy::copy)
      .def("calculate", &SelectorBase::calculate)

      .def("_reset", &SelectorBase::_reset, "The subclass reset operation implementation")
      .def("_calculate", &SelectorBase::_calculate, "[Overload interface] The subclass calculation interface")




      .def("get_selected", &SelectorBase::getSelected,
           R"(get_selected(self, datetime)

    [Overload interface] Get the selected system instances at the specified moment

    :param Datetime datetime: the specified moment
    :return: the list of the selected system instances
    :rtype: SystemList)")

      .def("add_sys", &SelectorBase::addSystem)
      .def("add_sys_list", &SelectorBase::addSystemList)

      .def(
        "set_scores_filter",
        [](SelectorBase& self, py::object filter) {
            py::gil_scoped_acquire gil;
            auto tmp = filter;
            self.setScoresFilter(filter.cast<ScoresFilterPtr>());
            tmp.release();
        },
        R"(set_scores_filter(self, filter)
           
    Set the ScoresFilter, which will replace the existing filter. It is only applicable to SE_MultiFactor
    
    :param ScoresFilter filter: ScoresFilter)")

      .def(
        "add_scores_filter",
        [](SelectorBase& self, py::object filter) {
            py::gil_scoped_acquire gil;
            auto tmp = filter;
            self.addScoresFilter(filter.cast<ScoresFilterPtr>());
            tmp.release();
        },
        R"(add_scores_filter(self, filter)
        
    Add a new filter on top of the existing one. It is only applicable to SE_MultiFactor    

    :param ScoresFilter filter: the new filter)")

      .def("__add__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self + other; })
      .def("__add__", [](const SelectorPtr& self, double other) { return self + other; })
      .def("__radd__", [](const SelectorPtr& self, double other) { return self + other; })

      .def("__sub__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self - other; })
      .def("__sub__", [](const SelectorPtr& self, double other) { return self - other; })
      .def("__rsub__", [](const SelectorPtr& self, double other) { return other - self; })

      .def("__mul__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self * other; })
      .def("__mul__", [](const SelectorPtr& self, double other) { return self * other; })
      .def("__rmul__", [](const SelectorPtr& self, double other) { return self * other; })

      .def("__truediv__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self / other; })
      .def("__truediv__", [](const SelectorPtr& self, double other) { return self / other; })
      .def("__rtruediv__", [](const SelectorPtr& self, double other) { return other / self; })

      .def("__and__",
           [](const SelectorPtr& self, const SelectorPtr& other) { return self & other; })
      .def("__or__", [](const SelectorPtr& self, const SelectorPtr& other) { return self | other; })

        DEF_PICKLE(SEPtr);

    m.def("SE_Fixed", [](double weight) { return SE_Fixed(weight); }, py::arg("weight") = 1.0);
    m.def(
      "SE_Fixed",
      [](const py::object& pystks, const SystemPtr& sys, double weight) {
          StockList stks = get_stock_list_from_python(pystks);
          return SE_Fixed(stks, sys, weight);
      },
      py::arg("stk_list"), py::arg("sys"), py::arg("weight") = 1.0,
      R"(SE_Fixed([stk_list, sys])

    The fixed selector, i.e. always selecting the initially defined targets and their system strategy prototypes

    :param list stk_list: the initially defined targets
    :param System sys: the system strategy prototype
    :param float weight: the default weight
    :return: the SE selector instance)");

    m.def("SE_Signal", py::overload_cast<>(SE_Signal));
    m.def("SE_Signal", py::overload_cast<const StockList&, const SystemPtr&>(SE_Signal),
          R"(SE_Signal([stk_list, sys])

    The signal selector, selecting only by the system buy signals

    :param list stk_list: the initially defined targets
    :param System sys: the system strategy prototype
    :return: the SE selector instance)");

    m.def("SE_MultiFactor", py::overload_cast<const MFPtr&, int>(SE_MultiFactor), py::arg("mf"),
          py::arg("topn") = 10);
    // Add the overloads supporting the FactorSet and the Indicator sequence
    m.def(
      "SE_MultiFactor",
      [](const py::object& input, int topn, int ic_n, int ic_rolling_n, const py::object& ref_stk,
         bool spearman, const string& mode) {
          Stock c_ref_stk = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return SE_MultiFactor(factset, topn, ic_n, ic_rolling_n, c_ref_stk, spearman, mode);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return SE_MultiFactor(c_inds, topn, ic_n, ic_rolling_n, c_ref_stk, spearman, mode);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("topn") = 10, py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      py::arg("ref_stk") = py::none(), py::arg("spearman") = true,
      py::arg("mode") = "MF_ICIRWeight",
      R"(SE_MultiFactor

    Create a multi-factor scoring based selector, supporting several creation ways

    - Specify the MF directly:
      :param MultiFactorBase mf: the directly specified multi-factor composition algorithm
      :param int topn: only select the first topn systems in the cross-section

    - Use a FactorSet:
      :param FactorSet input: the factor set
      :param int topn: only select the first topn systems in the cross-section; when it is less than or equal to 0, there is no limit
      :param int ic_n: the N-day return corresponding to the default IC
      :param int ic_rolling_n: the IC rolling period
      :param Stock ref_stk: the reference security, used for the date alignment; when unspecified, it is sh000001
      :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name

    - Use an Indicator sequence:
      :param sequense(Indicator) input: the original factor list
      :param int topn: only select the first topn systems in the cross-section; when it is less than or equal to 0, there is no limit
      :param int ic_n: the N-day return corresponding to the default IC
      :param int ic_rolling_n: the IC rolling period
      :param Stock ref_stk: the reference security, used for the date alignment; when unspecified, it is sh000001
      :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name

    .. code-block:: python
    
        # Use an Indicator list (the original way)
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)
        
        # Use a FactorSet (the new way)
        factor_set = FactorSet(indicators)
        selector2 = SE_MultiFactor(factor_set, topn=10)
        
        # Use the MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10))");

    m.def("SE_MultiFactor2", py::overload_cast<const MFPtr&, const SCFilterPtr&>(SE_MultiFactor2),
          py::arg("mf"), py::arg("filter") = SCFilter_IgnoreNan());
    // Add the overloads supporting the FactorSet and the Indicator sequence
    m.def(
      "SE_MultiFactor2",
      [](const py::object& input, int ic_n, int ic_rolling_n, const py::object& ref_stk,
         bool spearman, const string& mode, const SCFilterPtr& filter) {
          Stock c_ref_stk = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return SE_MultiFactor2(factset, ic_n, ic_rolling_n, c_ref_stk, spearman, mode,
                                     filter);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return SE_MultiFactor2(c_inds, ic_n, ic_rolling_n, c_ref_stk, spearman, mode, filter);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      py::arg("ref_stk") = py::none(), py::arg("spearman") = true,
      py::arg("mode") = "MF_ICIRWeight", py::arg("filter") = SCFilter_IgnoreNan(),
      R"(SE_MultiFactor2

    Create a multi-factor scoring based selector, supporting several creation ways

    - Specify the MF directly:
      :param MultiFactorBase mf: the directly specified multi-factor composition algorithm
      :param ScoresFilterBase filter: the scores filter

    - Use a FactorSet:
      :param FactorSet input: the factor set
      :param int ic_n: the N-day return corresponding to the default IC
      :param int ic_rolling_n: the IC rolling period
      :param Stock ref_stk: the reference security, used for the date alignment; when unspecified, it is sh000001
      :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name
      :param ScoresFilterBase filter: the scores filter

    - Use an Indicator sequence:
      :param sequense(Indicator) input: the original factor list
      :param int ic_n: the N-day return corresponding to the default IC
      :param int ic_rolling_n: the IC rolling period
      :param Stock ref_stk: the reference security, used for the date alignment; when unspecified, it is sh000001
      :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
      :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name
      :param ScoresFilterBase filter: the scores filter

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        selector2 = SE_MultiFactor2(factor_set)
        
        # Use the MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf))");

    m.def("crtSEOptimal", crtSEOptimal, R"(crtSEOptimal(func)
    
    Quickly create the optimization selector with a custom performance evaluation function

    :param func: a callable object, receiving the parameters (sys, lastdate) and returning a float value)");

    m.def("SE_MaxFundsOptimal", SE_MaxFundsOptimal, "The optimization selector maximizing the account assets");

    m.def("SE_PerformanceOptimal", SE_PerformanceOptimal, py::arg("key") = "Account Avg Annual Return %",
          py::arg("mode") = 0, R"(SE_PerformanceOptimal(key="Account Avg Annual Return %", mode=0)

    The selector optimizing by the Performance statistics results

    :param string key: the Performance statistics item
    :param int mode:  0 take the system with the maximum statistics result | 1 take the system with the minimum statistics result)");

    m.def(
      "SE_EvaluateOptimal",
      [](py::object evalulate_func) {
          py::object pyfunc = evalulate_func.attr("__call__");
          check_pyfunction_arg_num(pyfunc, 2);
          return SE_EvaluateOptimal([=](const SystemPtr& sys, const Datetime& enddate) {
              py::object pyfunc = evalulate_func.attr("__call__");
              return pyfunc(sys, enddate).cast<double>();
          });
      },
      R"(SE_EvaluateOptimal(evalulate_func)

    The selector optimizing with a custom function

    :param func: a callable object, receiving the parameters (sys, lastdate) and returning a float value)");
}