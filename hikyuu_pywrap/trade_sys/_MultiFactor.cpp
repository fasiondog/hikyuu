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
      .def_readwrite("stock", &ScoreRecord::stock, "证券")
      .def_readwrite("value", &ScoreRecord::value, "分值");

    m.def("scorerecords_to_np", [](const ScoreRecordList& scs) {
        size_t total = scs.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
            int32_t code[10];
            int32_t name[20];
            double value;
        };

        // 使用 malloc 分配内存
        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        std::string ucode, uname;
        for (size_t i = 0, len = scs.size(); i < len; i++) {
            const ScoreRecord& sc = scs[i];
            utf8_to_utf32(sc.stock.market_code(), data[i].code, 10);
            utf8_to_utf32(sc.stock.name(), data[i].name, 20);
            data[i].value = sc.value;
        }

        // 定义NumPy结构化数据类型
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>({htr("market_code"), htr("name"), htr("score")}),
                    vector_to_python_list<string>({"U10", "U20", "d"}),
                    vector_to_python_list<int64_t>({0, 40, 120}), sizeof(RawData));

        // 使用 capsule 管理内存
        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("scorerecords_to_df", [](const ScoreRecordList& scs) {
        size_t total = scs.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        // 创建 Python 字符串对象数组
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<double> value_arr(total);

        // 获取 value 数组缓冲区
        auto value_buf = value_arr.request();
        double* value_ptr = static_cast<double*>(value_buf.ptr);

        // 填充数据
        for (size_t i = 0; i < total; i++) {
            const ScoreRecord& sc = scs[i];
            code_list[i] = py::str(sc.stock.market_code());
            name_list[i] = py::str(sc.stock.name());
            value_ptr[i] = sc.value;
        }

        // 构建 DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("market_code").c_str()] =
          pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("name").c_str()] =
          pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns["score"] = value_arr;

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
    });

    py::class_<MultiFactorBase, MultiFactorPtr, PyMultiFactor>(m, "MultiFactorBase",
                                                               py::dynamic_attr(),
                                                               R"(市场环境判定策略基类

自定义市场环境判定策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())
      .def(py::init<const MultiFactorBase&>())

      .def("__str__", to_py_str<MultiFactorBase>)
      .def("__repr__", to_py_str<MultiFactorBase>)

      .def_property("name", py::overload_cast<>(&MultiFactorBase::name, py::const_),
                    py::overload_cast<const string&>(&MultiFactorBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("query", &MultiFactorBase::getQuery, &MultiFactorBase::setQuery,
                    py::return_value_policy::copy, R"(查询条件)")

      .def("get_param", &MultiFactorBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &MultiFactorBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &MultiFactorBase::haveParam, "是否存在指定参数")

      .def("get_ref_stock", &MultiFactorBase::getRefStock, py::return_value_policy::copy,
           "获取参考证券")
      .def("set_ref_stock", &MultiFactorBase::setRefStock, R"(set_ref_stock(self, stk)
      
    设置参考证券
    
    :param Stock stk: 参考证券)")

      .def("get_datetime_list", &MultiFactorBase::getDatetimeList, py::return_value_policy::copy,
           "获取参考日期列表（由参考证券通过查询条件获得）")

      .def("get_stock_list", &MultiFactorBase::getStockList, py::return_value_policy::copy,
           "获取创建时指定的证券列表")
      .def("set_stock_list", &MultiFactorBase::setStockList, R"(set_stock_list(self, stks)
      
    设置计算范围指定的证券列表
    
    :param list stks: 新的待计算证券列表)")

      .def("get_stock_list_num", &MultiFactorBase::getStockListNumber,
           "获取创建时指定的证券列表中证券数量")

      .def("get_ref_factorset", &MultiFactorBase::getRefFactorSet, py::return_value_policy::copy,
           "获取创建时输入的原始因子集合")

      .def("set_ref_factorset", &MultiFactorBase::setRefFactorSet,
           R"(set_ref_factorset(self, factorset)
      
    设置原始因子集合
    
    :param FactorSet factorset: 新的原始因子集合)")

      .def("get_factor", &MultiFactorBase::getFactor, py::return_value_policy::copy,
           py::arg("stock"), R"(get_factor(self, stock)

    获取指定证券合成后的新因子

    :param Stock stock: 指定证券)")

      .def("get_all_factors", &MultiFactorBase::getAllFactors, py::return_value_policy::copy,
           R"(get_all_factors(self)

    获取所有证券合成后的因子列表

    :return: [factor1, factor2, ...] 顺序与参考证券顺序相同)")

      .def(
        "set_normalize", [](PyMultiFactor& self, py::object norm) { self.set_norm(norm); },
        py::arg("norm"),
        R"(set_normalize(self, norm)

    设置标准化或归一化方法（影响全部因子）
    
    :param NormalizeBase norm: 标准化或归一化方法实例)")

      .def(
        "add_special_normalize",
        [](PyMultiFactor& self, const string& name, py::object norm, const string& category,
           const IndicatorList& style_inds) {
            self.add_special_norm(name, norm, category, style_inds);
        },
        py::arg("name"), py::arg("norm") = NormPtr(), py::arg("category") = "",
        py::arg("style_inds") = IndicatorList(),
        R"(add_special_normalize(self, name[, norm=None, category="", style_inds=[]])
        
    对指定名称的指标应用特定的标准化/归一化、行业中性化、风格因子中性化操作。标准化操作、行业中性化、风格因子中性化彼此无关，可同时指定也可分开指定。

    :param str name: 特殊归一化方法名称
    :param Normalize norm: 特殊归一化方法
    :param str category: 行业中性化时，指定板块类别
    :param list[Indicator] style_inds: 用于中性化的风格指标列表)")

      .def("get_ic", &MultiFactorBase::getIC, py::arg("ndays") = 0, R"(get_ic(self[, ndays=0])

    获取合成因子的IC, 长度与参考日期同

    ndays 对于使用 IC/ICIR 加权的新因子，最好保持好 ic_n 一致，
    但对于等权计算的新因子，不一定非要使用 ic_n 计算。
    所以，ndays 增加了一个特殊值 0, 表示直接使用 ic_n 参数计算 IC
     
    :rtype: Indicator)")

      .def("get_icir", &MultiFactorBase::getICIR, py::arg("ir_n"), py::arg("ic_n") = 0,
           R"(get_icir(self, ir_n[, ic_n=0])

    获取合成因子的 ICIR

    :param int ir_n: 计算 IR 的 n 窗口
    :param int ic_n: 计算 IC 的 n 窗口 (同 get_ic 中的 ndays))")

      .def("clone", &MultiFactorBase::clone, "克隆操作")

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

    获取指定日期截面的所有因子值，已经降序排列，相当于各证券日期截面评分。

    :param Datetime date: 指定日期
    :param int start: 取当日排名开始
    :param int end: 取当日排名结束(不包含本身)
    :param function func: (ScoreRecord)->bool 或 (Datetime, ScoreRecord)->bool 为原型的可调用对象
    :rtype: ScoreRecordList)")

      .def("get_all_scores", &MultiFactorBase::getAllScores, py::return_value_policy::copy,
           R"(get_all_scores(self)

    获取所有日期的所有评分，长度与参考日期相同

    :return: ScoreRecordList)")

      .def("get_all_src_factors", &MultiFactorBase::getAllSrcFactors, R"(get_all_src_factors(self)

    获取所有原始因子列表(如果指定了标准化、行业中性化, 返回为已处理的因子列表)

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
          
          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_EqualWeight(factset, c_stks, query, ref_stock, ic_n, spearman, mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_EqualWeight(c_inds, c_stks, query, ref_stock, ic_n, spearman, mode, save_all_factors);
          } else {
              throw std::invalid_argument("First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("spearman") = true, py::arg("mode") = 0,
      py::arg("save_all_factors") = false,
      R"(MF_EqualWeight(input, stks, query, ref_stk[, ic_n=5])

    等权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券用于日期对齐 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)
        
        # 使用FactorSet
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
          
          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_Weight(factset, c_weights, c_stks, query, ref_stock, ic_n, spearman, mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_Weight(c_inds, c_weights, c_stks, query, ref_stock, ic_n, spearman, mode, save_all_factors);
          } else {
              throw std::invalid_argument("First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("weights"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    按指定权重合成因子 = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequense(stock) stks: 计算证券列表
    :param sequense(float) weights: 权重列表(需和因子数量等长)
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券用于日期对齐 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        weights = [0.6, 0.4]
        mf1 = MF_Weight(indicators, stocks, weights, query)
        
        # 使用FactorSet
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
          
          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_ICWeight(factset, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_ICWeight(c_inds, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
          } else {
              throw std::invalid_argument("First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动IC权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 用于日期对齐的参考证券 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)
        
        # 使用FactorSet
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
          
          // 判断输入类型
          if (py::isinstance<FactorSet>(input)) {
              // 输入是FactorSet
              FactorSet factset = input.cast<FactorSet>();
              return MF_ICIRWeight(factset, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
          } else if (py::isinstance<py::sequence>(input)) {
              // 输入是序列（假设为Indicator列表）
              IndicatorList c_inds = python_list_to_vector<Indicator>(input);
              return MF_ICIRWeight(c_inds, c_stks, query, ref_stock, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
          } else {
              throw std::invalid_argument("First parameter must be either FactorSet or sequence of Indicator");
          }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动ICIR权重合成因子，支持多种输入类型

    :param input: 因子输入，可以是FactorSet对象或Indicator序列
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 用于日期对齐的参考证券 (未指定时，默认为 sh000001)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :param bool spearman: 默认使用 spearman 计算相关系数，否则为 pearson
    :param int mode: 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    :param bool save_all_factors: 是否保存所有因子值,影响 get_actor/get_all_factors 方法
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # 使用Indicator列表
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICIRWeight(indicators, stocks, query)
        
        # 使用FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICIRWeight(factor_set, stocks, query))");
}