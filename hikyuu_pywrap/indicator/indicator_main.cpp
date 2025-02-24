/*
 * indicator_main.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
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
            parallel_for_index(0, cinds.size(), [&](size_t i) { return cinds[i](kdata); }));
          return ret;
      },
      R"(batch_calculate_inds(inds, kdata) -> list)
    
    并行计算多个指标
    
    :param list inds: 指标列表
    :param KData kdata: K线数据
    :return: 指标计算结果列表
    :rtype: list)");
}
