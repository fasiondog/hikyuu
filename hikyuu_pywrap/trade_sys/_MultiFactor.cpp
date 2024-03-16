/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/factor/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyMultiFactor : public MultiFactorBase {
    PY_CLONE(PyMultiFactor, MultiFactorBase)

public:
    using MultiFactorBase::MultiFactorBase;

    IndicatorList _calculate(const vector<IndicatorList>& all_stk_inds) {
        PYBIND11_OVERLOAD_PURE_NAME(IndicatorList, MultiFactorBase, "_calculate", _calculate,
                                    all_stk_inds);
    }
};

void export_MultiFactor(py::module& m) {
    py::class_<MultiFactorBase, MultiFactorPtr, PyMultiFactor>(m, "MultiFactor",
                                                               R"(市场环境判定策略基类

自定义市场环境判定策略接口：

    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")
      .def(py::init<>())

      .def("__str__", to_py_str<MultiFactorBase>)
      .def("__repr__", to_py_str<MultiFactorBase>)

      .def_property("name", py::overload_cast<>(&MultiFactorBase::name, py::const_),
                    py::overload_cast<const string&>(&MultiFactorBase::name),
                    py::return_value_policy::copy, "名称")
      .def("get_query", &MultiFactorBase::getQuery, py::return_value_policy::copy)

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

      .def("get_factor", &MultiFactorBase::getFactor, py::return_value_policy::copy)

      .def("get_all_factors",
           [](MultiFactorBase& self) {
               // return vector_to_python_list<Indicator>()
               auto factors = self.getAllFactors();
               IndicatorList copy_factors;
               copy_factors.reserve(factors.size());
               for (const auto& factor : factors) {
                   copy_factors.emplace_back(factor.clone());
               }
               return vector_to_python_list<Indicator>(copy_factors);
           })

      .def("get_ic", &MultiFactorBase::getIC, py::arg("ndays") = 0)
      .def("get_icir", &MultiFactorBase::getICIR, py::arg("ir_n"), py::arg("ic_n") = 0)
      .def("clone", &MultiFactorBase::clone)

      .def("get_cross",
           [](MultiFactorBase& self, const Datetime& date) {
               py::list ret;
               auto cross = self.getCross(date);
               for (const auto& item : cross) {
                   ret.append(py::make_tuple(item.first, item.second));
               }
               return ret;
           })

      .def("get_all_cross",
           [](MultiFactorBase& self) {
               py::list ret;
               auto all_cross = self.getAllCross();
               for (const auto& one_day : all_cross) {
                   py::list one;
                   for (const auto& item : one_day) {
                       one.append(py::make_tuple(item.first, item.second));
                   }
                   ret.append(std::move(one));
               }
               return ret;
           })

        DEF_PICKLE(MultiFactorPtr);

    m.def(
      "MF_EqualWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const Stock& ref_stk, int ic_n) {
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_EqualWeight(c_inds, c_stks, query, ref_stk, ic_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk"), py::arg("ic_n") = 5,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5])

    等权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :rtype: MultiFactorPtr)");

    m.def(
      "MF_ICWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const Stock& ref_stk, int ic_n, int ic_rolling_n) {
          // MF_EqualWeight
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_ICWeight(c_inds, c_stks, query, ref_stk, ic_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk"), py::arg("ic_n") = 5,
      py::arg("ic_rolling_n") = 120,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动IC权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :rtype: MultiFactorPtr)");

    m.def(
      "MF_ICIRWeight",
      [](const py::sequence& inds, const py::sequence& stks, const KQuery& query,
         const Stock& ref_stk, int ic_n, int ic_rolling_n) {
          // MF_EqualWeight
          IndicatorList c_inds = python_list_to_vector<Indicator>(inds);
          StockList c_stks = python_list_to_vector<Stock>(stks);
          return MF_ICIRWeight(c_inds, c_stks, query, ref_stk, ic_n);
      },
      py::arg("inds"), py::arg("stks"), py::arg("query"), py::arg("ref_stk"), py::arg("ic_n") = 5,
      py::arg("ic_rolling_n") = 120,
      R"(MF_EqualWeight(inds, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    滚动ICIR权重合成因子

    :param sequense(Indicator) inds: 原始因子列表
    :param sequense(stock) stks: 计算证券列表
    :param Query query: 日期范围
    :param Stock ref_stk: 参考证券
    :param int ic_n: 默认 IC 对应的 N 日收益率
    :param int ic_rolling_n: IC 滚动周期
    :rtype: MultiFactorPtr)");
}