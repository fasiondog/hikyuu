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

class PyMultiFactor : public MultiFactorBase {
    PY_CLONE(PyMultiFactor, MultiFactorBase)

public:
    using MultiFactorBase::MultiFactorBase;
    PyMultiFactor(const MultiFactorBase& base) : MultiFactorBase(base) {}

    IndicatorList _calculate(const vector<IndicatorList>& all_stk_inds) {
        // PYBIND11_OVERLOAD_PURE_NAME(IndicatorList, MultiFactorBase, "_calculate", _calculate,
        //                             all_stk_inds);
        auto self = py::cast(this);
        auto func = self.attr("_calculate")();
        auto py_all_stk_inds = vector_to_python_list<IndicatorList>(all_stk_inds);
        auto py_ret = func(py_all_stk_inds);
        return py_ret.cast<IndicatorList>();
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

      .def("get_ref_indicators", &MultiFactorBase::getRefIndicators, py::return_value_policy::copy,
           "获取创建时输入的原始因子列表")

      .def("set_ref_indicators", &MultiFactorBase::setRefIndicators,
           R"(set_ref_indicators(self, inds)
      
    设置原始因子列表
    
    :param list inds: 新的原始因子列表)")

      .def("get_factor", &MultiFactorBase::getFactor, py::return_value_policy::copy,
           py::arg("stock"), R"(get_factor(self, stock)

    获取指定证券合成后的新因子

    :param Stock stock: 指定证券)")

      .def("get_all_factors", &MultiFactorBase::getAllFactors, py::return_value_policy::copy,
           R"(get_all_factors(self)

    获取所有证券合成后的因子列表

    :return: [factor1, factor2, ...] 顺序与参考证券顺序相同)")

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

      .def("get_all_src_factors", &MultiFactorBase::getAllSrcFactors)

        DEF_PICKLE(MultiFactorPtr);

    m.def("MF_EqualWeight", py::overload_cast<>(MF_EqualWeight));
    m.def(
      "MF_EqualWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n) {
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_EqualWeight(c_inds, c_stks, query,
                                ref_stk.is_none() ? getStock("sh000300") : ref_stk.cast<Stock>(),
                                ic_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5])

    等权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :rtype: MultiFactor)");

    m.def("MF_ICWeight", py::overload_cast<>(MF_ICWeight));
    m.def(
      "MF_ICWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n) {
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_ICWeight(c_inds, c_stks, query,
                             ref_stk.is_none() ? getStock("sh000300") : ref_stk.cast<Stock>(), ic_n,
                             ic_rolling_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动IC权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk:  (未指定时，默认为 sh000300 沪深300)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :rtype: MultiFactor)");

    m.def("MF_ICIRWeight", py::overload_cast<>(MF_ICIRWeight));
    m.def(
      "MF_ICIRWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n) {
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_ICIRWeight(c_inds, c_stks, query,
                               ref_stk.is_none() ? getStock("sh000300") : ref_stk.cast<Stock>(),
                               ic_n, ic_rolling_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk") = py::none(),
      py::arg("ic_n") = 5, py::arg("ic_rolling_n") = 120,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动ICIR权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券 (未指定时，默认为 sh000300 沪深300)
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :rtype: MultiFactor)");
}