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
    
    Calculate multiple indicators in parallel
    
    :param list inds: the indicator list
    :param KData kdata: the K-line data
    :return: the list of the indicator calculation results
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
    
    Perform a multiple linear regression analysis on the stock, using the return of the stock close price as the dependent variable
    
    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition
    :param Indicator *inds: one or more indicators as the independent variables
    :return: the list of the regression coefficients; the first element is alpha (the intercept), followed by each beta coefficient
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
    
    Perform a multiple linear regression analysis on the stock (the full version), returning the complete regression result
    
    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition
    :param Indicator *inds: one or more indicators as the independent variables
    :return: the list of the regression results, in the format:
             [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
             - alpha: the intercept
             - beta1~betan: the coefficient of each factor
             - e1~en: the residual of each data point (the actual value - the predicted value)
             - RSS: the residual sum of squares
             - R²: the coefficient of determination
    :rtype: list
    :example:
    
        >>> stk = getStock('sh000001')
        >>> result = multi_regression_full(stk, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> residuals = result[3:-2]  # the residual sequence
        >>> RSS = result[-2]
        >>> R_squared = result[-1]
    )");
}