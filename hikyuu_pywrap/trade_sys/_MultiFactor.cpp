/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/multifactor/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#ifdef _MSC_VER
#define HIDDEN
#else
#define HIDDEN __attribute__((visibility("hidden")))
#endif

class HIDDEN PyMultiFactor : public MultiFactorBase {
    PY_CLONE(PyMultiFactor, MultiFactorBase)

public:
    PyMultiFactor() : MultiFactorBase() {
        m_is_python_object = true;
    }

    PyMultiFactor(const string& name) : MultiFactorBase(name) {
        m_is_python_object = true;
    }

    PyMultiFactor(const MultiFactorBase& base) : MultiFactorBase(base) {
        m_is_python_object = true;
    }

    virtual ~PyMultiFactor() override {}

    IndicatorList _calculate(const vector<IndicatorList>& all_stk_inds) override {
        // PYBIND11_OVERLOAD_PURE_NAME(IndicatorList, MultiFactorBase, "_calculate", _calculate,
        //                             all_stk_inds);
        auto self = py::cast(this);
        auto func = self.attr("_calculate")();
        auto py_all_stk_inds = vector_to_python_list<IndicatorList>(all_stk_inds);
        auto py_ret = func(py_all_stk_inds);
        return py_ret.cast<IndicatorList>();
    }

public:
    void set_norm(py::object norm) {
        py::gil_scoped_acquire gil;
        auto tmp = norm;
        setNormalize(norm.cast<NormPtr>());
        tmp.release();
    }

    void add_special_norm(const string& name, py::object norm, const string& category,
                          const IndicatorList& style_inds) {
        py::gil_scoped_acquire gil;
        HKU_INFO_IF_RETURN(!norm || norm.is_none(), void(), "norm is None");
        auto tmp = norm;
        addSpecialNormalize(name, norm.cast<NormPtr>(), category, style_inds);
        tmp.release();
    }
};

void export_MultiFactor(py::module& m) {
    py::class_<ScoreRecord>(m, "ScoreRecord", "")
      .def(py::init<>())
      .def(py::init<const Stock&, ScoreRecord::value_t>())
      .def("__str__", to_py_str<ScoreRecord>)
      .def("__repr__", to_py_str<ScoreRecord>)
      .def_readwrite("stock", &ScoreRecord::stock, "The security")
      .def_readwrite("value", &ScoreRecord::value, "The score");

    m.def("scorerecords_to_np", [](const ScoreRecordList& scs) {
        size_t total = scs.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
            int32_t code[10];
            int32_t name[20];
            double value;
        };

        // Allocate the memory with malloc
        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        std::string ucode, uname;
        for (size_t i = 0, len = scs.size(); i < len; i++) {
            const ScoreRecord& sc = scs[i];
            utf8_to_utf32(sc.stock.market_code(), data[i].code, 10);
            utf8_to_utf32(sc.stock.name(), data[i].name, 20);
            data[i].value = sc.value;
        }

        // Define the NumPy structured data type
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>({htr("market_code"), htr("name"), htr("score")}),
                    vector_to_python_list<string>({"U10", "U20", "d"}),
                    vector_to_python_list<int64_t>({0, 40, 120}), sizeof(RawData));

        // Manage the memory with the capsule
        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("scorerecords_to_df", [](const ScoreRecordList& scs) {
        size_t total = scs.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the python string object array
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<double> value_arr(total);

        // Get the buffer of the value array
        auto value_buf = value_arr.request();
        double* value_ptr = static_cast<double*>(value_buf.ptr);

        // Fill the data
        for (size_t i = 0; i < total; i++) {
            const ScoreRecord& sc = scs[i];
            code_list[i] = py::str(sc.stock.market_code());
            name_list[i] = py::str(sc.stock.name());
            value_ptr[i] = sc.value;
        }

        // Build the DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("market_code").c_str()] =
          pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("name").c_str()] =
          pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns["score"] = value_arr;

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
    });

    py::class_<MultiFactorBase, MultiFactorPtr, PyMultiFactor>(
      m, "MultiFactorBase", py::dynamic_attr(),
      R"(The market environment strategy base class

The custom market environment strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const MultiFactorBase&>())

      .def("__str__", to_py_str<MultiFactorBase>)
      .def("__repr__", to_py_str<MultiFactorBase>)

      .def_property("name", py::overload_cast<>(&MultiFactorBase::name, py::const_),
                    py::overload_cast<const string&>(&MultiFactorBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &MultiFactorBase::getQuery, &MultiFactorBase::setQuery,
                    py::return_value_policy::copy, R"(The query condition)")

      .def("get_param", &MultiFactorBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (MultiFactorBase::*)(const std::string&, const boost::any&)>(
             &MultiFactorBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &MultiFactorBase::haveParam, "Whether the specified parameter exists")

      .def("get_ref_stock", &MultiFactorBase::getRefStock, py::return_value_policy::copy,
           "Get the reference security")
      .def("set_ref_stock", &MultiFactorBase::setRefStock, R"(set_ref_stock(self, stk)
      
    Set the reference security
    
    :param Stock stk: the reference security)")

      .def("get_datetime_list", &MultiFactorBase::getDatetimeList, py::return_value_policy::copy,
           "Get the reference date list (obtained from the reference security through the query "
           "condition)")

      .def("get_stock_list", &MultiFactorBase::getStockList, py::return_value_policy::copy,
           "Get the security list specified at the creation")
      .def("set_stock_list", &MultiFactorBase::setStockList, R"(set_stock_list(self, stks)
      
    Set the security list specified for the calculation range
    
    :param list stks: the new security list to calculate)")

      .def("get_stock_list_num", &MultiFactorBase::getStockListNumber,
           "Get the number of the securities in the security list specified at the creation")

      .def("get_ref_factorset", &MultiFactorBase::getRefFactorSet, py::return_value_policy::copy,
           "Get the original factor set input at the creation")

      .def(
        "set_ref_factorset",
        [](MultiFactorBase& self, const py::sequence& inds, const KQuery::KType& ktype) {
            IndicatorList ind_list = python_list_to_vector<Indicator>(inds);
            FactorSet factorset = FactorSet(ind_list, ktype);
            self.setRefFactorSet(factorset);
        },
        py::arg("inds"), py::arg("ktype") = KQuery::DAY)
      .def(
        "set_ref_factorset",
        [](MultiFactorBase& self, const py::dict& inds, const KQuery::KType& ktype) {
            std::unordered_map<string, Indicator> inds_dict;
            for (auto iter = inds.begin(); iter != inds.end(); ++iter) {
                inds_dict[iter->first.cast<string>()] = iter->second.cast<Indicator>();
            }
            FactorSet factorset = FactorSet(inds_dict, ktype);
            self.setRefFactorSet(factorset);
        },
        py::arg("inds"), py::arg("ktype") = KQuery::DAY)
      .def(
        "set_ref_factorset",
        [](MultiFactorBase& self, FactorSet factorset) { self.setRefFactorSet(factorset); },
        R"(set_ref_factorset(self, factorset)
      
    Set the original factor set
    
    :param FactorSet factorset: the new original factor set)")

      .def("get_factor", &MultiFactorBase::getFactor, py::return_value_policy::copy,
           py::arg("stock"), R"(get_factor(self, stock)

    Get the new composed factor of the specified security

    :param Stock stock: the specified security)")

      .def("get_all_factors", &MultiFactorBase::getAllFactors, py::return_value_policy::copy,
           R"(get_all_factors(self)

    Get the list of the composed factors of all the securities

    :return: [factor1, factor2, ...] in the same order as the reference securities)")

      .def(
        "set_normalize", [](PyMultiFactor& self, py::object norm) { self.set_norm(norm); },
        py::arg("norm"),
        R"(set_normalize(self, norm)

    Set the standardization or normalization method (affecting all the factors)
    
    :param NormalizeBase norm: the standardization or normalization method instance)")

      .def(
        "add_special_normalize",
        [](PyMultiFactor& self, const string& name, py::object norm, const string& category,
           const IndicatorList& style_inds) {
            self.add_special_norm(name, norm, category, style_inds);
        },
        py::arg("name"), py::arg("norm") = NormPtr(), py::arg("category") = "",
        py::arg("style_inds") = IndicatorList(),
        R"(add_special_normalize(self, name[, norm=None, category="", style_inds=[]])
        
    Apply a specific standardization/normalization, industry neutralization or style factor neutralization operation to the indicator with the specified name. The standardization operation, the industry neutralization and the style factor neutralization are independent of each other; they can be specified together or separately.

    :param str name: the special normalization method name
    :param Normalize norm: the special normalization method
    :param str category: for the industry neutralization, specify the block category
    :param list[Indicator] style_inds: the list of the style indicators used for the neutralization)")

      .def("get_ic", &MultiFactorBase::getIC, py::arg("ndays") = 0, R"(get_ic(self[, ndays=0])

    Get the IC of the composed factor, with the same length as the reference dates

    For the new factors weighted with IC/ICIR, it is best to keep ndays consistent with ic_n,
    but for the new factors calculated with the equal weights, it is not necessarily required to calculate with ic_n.
    Therefore, ndays has a special value 0, which means calculating the IC directly with the ic_n parameter
     
    :rtype: Indicator)")

      .def("get_icir", &MultiFactorBase::getICIR, py::arg("ir_n"), py::arg("ic_n") = 0,
           R"(get_icir(self, ir_n[, ic_n=0])

    Get the ICIR of the composed factor

    :param int ir_n: the n window for calculating the IR
    :param int ic_n: the n window for calculating the IC (the same as ndays in get_ic))")

      .def("clone", &MultiFactorBase::clone, "The clone operation")

      .def(
        "get_scores",
        [](MultiFactorBase& self, const Datetime& date, size_t start, py::object end,
           py::object filter) {
            size_t cend = end.is_none() ? Null<size_t>() : end.cast<size_t>();
            if (filter.is_none()) {
                return self.getScores(date, start, cend, std::function<bool(const ScoreRecord&)>());
            }
            HKU_CHECK(py::hasattr(filter, "__call__"), "filter not callable!");
            py::object filter_func = filter.attr("__call__");
            ScoreRecord sc;
            try {
                filter_func(sc);
                return self.getScores(date, start, cend, [&](const ScoreRecord& score_) {
                    return filter_func(score_).cast<bool>();
                });
            } catch (...) {
                filter_func(date, sc);
                return self.getScores(date, start, cend,
                                      [&](const Datetime& date_, const ScoreRecord& score_) {
                                          return filter_func(date_, score_).cast<bool>();
                                      });
            }
        },
        py::arg("date"), py::arg("start") = 0, py::arg("end") = py::none(),
        py::arg("filter") = py::none(),
        R"(get_score(self, date[, start=0, end=Null])

    Get all the factor values of the cross-section on the specified date, already sorted descending, equivalent to the cross-section scores of the securities on that date.

    :param Datetime date: the specified date
    :param int start: the start of the daily ranking to take
    :param int end: the end of the daily ranking to take (exclusive)
    :param function filter: a callable object with the prototype (ScoreRecord)->bool or (Datetime, ScoreRecord)->bool
    :rtype: ScoreRecordList)")

      .def("get_all_scores", &MultiFactorBase::getAllScores, py::return_value_policy::copy,
           R"(get_all_scores(self)

    Get all the scores of all the dates, with the same length as the reference dates

    :return: ScoreRecordList)")

      .def("get_all_src_factors", &MultiFactorBase::getAllSrcFactors, R"(get_all_src_factors(self)

    Get the list of all the original factors (if the standardization or the industry neutralization is specified, the returned list is the processed factor list)

    :rtype: list
    :return: list IndicatorList stks x inds)")

        DEF_PICKLE(MultiFactorPtr);

    m.def("MF_EqualWeight", py::overload_cast<>(MF_EqualWeight));
    m.def(
      "MF_EqualWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, bool spearman, int mode, bool save_all_factors) {
          StockList c_stks = get_stock_list_from_python(stks);
          Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_EqualWeight(factset, c_stks, query, ref_stock, ic_n, spearman, mode,
                                    save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_EqualWeight(c_inds, c_stks, query, ref_stock, ic_n, spearman, mode,
                                    save_all_factors);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("spearman") = true, py::arg("mode") = 0,
      py::arg("save_all_factors") = false,
      R"(MF_EqualWeight(input, stks, query, ref_stk[, ic_n=5])

    Compose the factor with the equal weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequense(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_EqualWeight(factor_set, stocks, query))");

    m.def("MF_Weight", py::overload_cast<>(MF_Weight));
    m.def(
      "MF_Weight",
      [](const py::object& input, const py::object& stks, const py::object& weights_obj,
         const KQuery& query, const py::object& ref_stk, int ic_n, bool spearman, int mode,
         bool save_all_factors) {
          StockList c_stks = get_stock_list_from_python(stks);
          Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();
          PriceList c_weights = python_list_to_vector<price_t>(weights_obj);

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_Weight(factset, c_weights, c_stks, query, ref_stock, ic_n, spearman, mode,
                               save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_Weight(c_inds, c_weights, c_stks, query, ref_stock, ic_n, spearman, mode,
                               save_all_factors);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("weights"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    Compose the factor by the specified weights = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequense(stock) stks: the list of the securities to calculate
    :param sequense(float) weights: the weight list (must be the same length as the number of the factors)
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        weights = [0.6, 0.4]
        mf1 = MF_Weight(indicators, stocks, weights, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_Weight(factor_set, stocks, weights, query))");

    m.def("MF_ICWeight", py::overload_cast<>(MF_ICWeight));
    m.def(
      "MF_ICWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n, bool spearman, int mode,
         bool save_all_factors) {
          StockList c_stks = get_stock_list_from_python(stks);
          Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_ICWeight(factset, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman,
                                 mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_ICWeight(c_inds, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman,
                                 mode, save_all_factors);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling IC weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequense(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICWeight(factor_set, stocks, query))");

    m.def("MF_ICIRWeight", py::overload_cast<>(MF_ICIRWeight));
    m.def(
      "MF_ICIRWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n, bool spearman, int mode,
         bool save_all_factors) {
          StockList c_stks = get_stock_list_from_python(stks);
          Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

          // Judge the input type
          if (py::isinstance<FactorSet>(input)) {
              // The input is a FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_ICIRWeight(factset, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman,
                                   mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // The input is a sequence (assumed to be an Indicator list)
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_ICIRWeight(c_inds, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman,
                                   mode, save_all_factors);
          } else {
              throw std::invalid_argument(
                "First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling ICIR weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequense(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICIRWeight(indicators, stocks, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICIRWeight(factor_set, stocks, query))");
}