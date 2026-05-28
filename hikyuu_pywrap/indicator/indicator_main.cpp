/*
 * indicator_main.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/indicator/utils.h>
#include <hikyuu/utilities/thread/algorithm.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_Indicator(py::module& m);
void export_IndicatorImp(py::module& m);
void export_IndParam(py::module& m);
void export_Indicator_build_in(py::module& m);
void export_Indicator_ta_lib(py::module& m);

void export_indicator_main(py::module& m) {
    export_Indicator(m);
    export_IndicatorImp(m);
    export_IndParam(m);
    export_Indicator_build_in(m);
    export_Indicator_ta_lib(m);

    m.def(
      "batch_calculate_inds",
      [](const py::sequence& inds, const KData& kdata) {
          py::list ret;
          HKU_IF_RETURN(len(inds) == 0, ret);
          IndicatorList cinds = python_list_to_vector<Indicator>(inds);
          ret = vector_to_python_list(
            global_parallel_for_index(0, cinds.size(), [&](size_t i) { return cinds[i](kdata); }));
          return ret;
      },
      R"(batch_calculate_inds(inds, kdata) -> list)
    
    并行计算多个指标
    
    :param list inds: 指标列表
    :param KData kdata: K线数据
    :return: 指标计算结果列表
    :rtype: list)");

    m.def(
      "multi_regression",
      [](const Stock& stk, const KQuery& query, const py::args& inds) {
          IndicatorList cinds;
          for (const auto& ind : inds) {
              cinds.push_back(ind.cast<Indicator>());
          }
          return multi_regression(stk, query, cinds);
      },
      R"(multi_regression(stk, query, *inds) -> list)
    
    对股票进行多元线性回归分析，使用股票收盘价的收益率作为因变量
    
    :param Stock stk: 股票对象
    :param KQuery query: K线查询条件
    :param Indicator *inds: 一个或多个指标作为自变量
    :return: 回归系数列表，第一个元素是alpha(截距)，后续是各个beta系数
    :rtype: list
    :example:
    
        >>> stk = getStock('sh000001')
        >>> result = multi_regression(stk, KQuery(-252), MA(CLOSE(), 5), MACD(CLOSE())[0], RSI(CLOSE(), 14))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> beta3 = result[3]
    )");

    m.def(
      "multi_regression_full",
      [](const Stock& stk, const KQuery& query, const py::args& inds) {
          IndicatorList cinds;
          for (const auto& ind : inds) {
              cinds.push_back(ind.cast<Indicator>());
          }
          return multi_regression_full(stk, query, cinds);
      },
      R"(multi_regression_full(stk, query, *inds) -> list)
    
    对股票进行多元线性回归分析（完整版本），返回完整的回归结果
    
    :param Stock stk: 股票对象
    :param KQuery query: K线查询条件
    :param Indicator *inds: 一个或多个指标作为自变量
    :return: 回归结果列表，格式为：
             [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
             - alpha: 截距项
             - beta1~betan: 各因子系数
             - e1~en: 各数据点的残差（实际值-预测值）
             - RSS: 残差平方和
             - R²: 决定系数
    :rtype: list
    :example:
    
        >>> stk = getStock('sh000001')
        >>> result = multi_regression_full(stk, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> residuals = result[3:-2]  # 残差序列
        >>> RSS = result[-2]
        >>> R_squared = result[-1]
    )");
}