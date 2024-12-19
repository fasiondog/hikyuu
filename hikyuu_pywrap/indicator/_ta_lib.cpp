/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator_talib/ta_crt.h>

#if HKU_ENABLE_TA_LIB

#include "../pybind_utils.h"

#define TA_IN1_OUT1_N_PY(func, period)                                            \
    m.def(#func, py::overload_cast<int>(func), py::arg("n") = period);            \
    m.def(#func, py::overload_cast<const IndParam&>(func));                       \
    m.def(#func, py::overload_cast<const Indicator&, const IndParam&>(func));     \
    m.def(#func, py::overload_cast<const Indicator&, const Indicator&>(func));    \
    m.def(#func, py::overload_cast<const Indicator&, int>(func), py::arg("data"), \
          py::arg("n") = period);

namespace py = pybind11;
using namespace hku;

void export_Indicator_ta_lib(py::module& m) {
    TA_IN1_OUT1_N_PY(TA_CMO, 14)
    TA_IN1_OUT1_N_PY(TA_DEMA, 30)
    TA_IN1_OUT1_N_PY(TA_EMA, 30)
    TA_IN1_OUT1_N_PY(TA_AVGDEV, 14)
    TA_IN1_OUT1_N_PY(TA_KAMA, 30)
    TA_IN1_OUT1_N_PY(TA_LINEARREG_ANGLE, 14)
    TA_IN1_OUT1_N_PY(TA_MAX, 30)
    TA_IN1_OUT1_N_PY(TA_MIDPOINT, 14)
    TA_IN1_OUT1_N_PY(TA_MIN, 30)
    TA_IN1_OUT1_N_PY(TA_MOM, 10)
    TA_IN1_OUT1_N_PY(TA_ROC, 10)
    TA_IN1_OUT1_N_PY(TA_ROCP, 10)
    TA_IN1_OUT1_N_PY(TA_ROCR, 10)
    TA_IN1_OUT1_N_PY(TA_ROCR100, 10)
    TA_IN1_OUT1_N_PY(TA_RSI, 14)
    TA_IN1_OUT1_N_PY(TA_SMA, 30)
    TA_IN1_OUT1_N_PY(TA_TEMA, 30)  //?
    TA_IN1_OUT1_N_PY(TA_TRIMA, 30)
    TA_IN1_OUT1_N_PY(TA_TRIX, 30)
    TA_IN1_OUT1_N_PY(TA_TSF, 14)
    TA_IN1_OUT1_N_PY(TA_WMA, 30)
}

#endif /* HKU_ENABLE_TA_LIB */