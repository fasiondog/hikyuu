/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator_talib/ta_crt.h>

#if HKU_ENABLE_TA_LIB

#include "../pybind_utils.h"

#define TA_IN1_OUT_PY(func)                                  \
    m.def(#func, py::overload_cast<>(func));                 \
    m.def(#func, py::overload_cast<const Indicator&>(func)); \
    m.def(#func, py::overload_cast<Indicator::value_t>(func));

#define TA_IN1_OUT_N_PY(func, period)                                             \
    m.def(#func, py::overload_cast<int>(func), py::arg("n") = period);            \
    m.def(#func, py::overload_cast<const IndParam&>(func));                       \
    m.def(#func, py::overload_cast<const Indicator&, const IndParam&>(func));     \
    m.def(#func, py::overload_cast<const Indicator&, const Indicator&>(func));    \
    m.def(#func, py::overload_cast<const Indicator&, int>(func), py::arg("data"), \
          py::arg("n") = period);

#define TA_K_OUT_PY(func)                    \
    m.def(#func, py::overload_cast<>(func)); \
    m.def(#func, py::overload_cast<const KData&>(func), py::arg("data"));

#define TA_K_OUT_N_PY(func, period)                                           \
    m.def(#func, py::overload_cast<int>(func), py::arg("n") = period);        \
    m.def(#func, py::overload_cast<const KData&, int>(func), py::arg("data"), \
          py::arg("n") = period);

namespace py = pybind11;
using namespace hku;

void export_Indicator_ta_lib(py::module& m) {
    TA_K_OUT_N_PY(TA_ACCBANDS, 20)
    TA_IN1_OUT_PY(TA_ACOS)
    TA_K_OUT_PY(TA_AD)

    m.def("TA_ADOSC", py::overload_cast<int, int>(TA_ADOSC), py::arg("fast_n") = 3,
          py::arg("slow_n") = 10);
    m.def("TA_ADOSC", py::overload_cast<const KData&, int, int>(TA_ADOSC), py::arg("data"),
          py::arg("fast_n") = 3, py::arg("slow_n") = 10);

    TA_K_OUT_N_PY(TA_ADX, 14)
    TA_K_OUT_N_PY(TA_ADXR, 14)

    m.def("TA_APO", py::overload_cast<int, int, int>(TA_APO), py::arg("fast_n") = 12,
          py::arg("slow_n") = 26, py::arg("matype") = 0);
    m.def("TA_APO", py::overload_cast<const Indicator&, int, int, int>(TA_APO), py::arg("data"),
          py::arg("fast_n") = 12, py::arg("slow_n") = 26, py::arg("matype") = 0);

    TA_K_OUT_N_PY(TA_AROON, 14)
    TA_K_OUT_N_PY(TA_AROONOSC, 14)

    TA_IN1_OUT_PY(TA_ASIN)
    TA_IN1_OUT_PY(TA_ATAN)

    TA_K_OUT_N_PY(TA_ATR, 14)

    TA_IN1_OUT_N_PY(TA_AVGDEV, 14)
    TA_K_OUT_PY(TA_AVGPRICE)

    m.def("TA_BBANDS", py::overload_cast<int, double, double, int>(TA_BBANDS), py::arg("n") = 5,
          py::arg("nbdevup") = 2., py::arg("nbdevdn") = 2., py::arg("matype") = 0);
    m.def("TA_BBANDS", py::overload_cast<const Indicator&, int, double, double, int>(TA_BBANDS),
          py::arg("data"), py::arg("n") = 5, py::arg("nbdevup") = 2., py::arg("nbdevdn") = 2.,
          py::arg("matype") = 0);

    TA_K_OUT_PY(TA_BOP)
    TA_K_OUT_N_PY(TA_CCI, 14)
    TA_K_OUT_PY(TA_CDL2CROWS)
    TA_K_OUT_PY(TA_CDL3BLACKCROWS)
    TA_K_OUT_PY(TA_CDL3INSIDE)
    TA_K_OUT_PY(TA_CDL3LINESTRIKE)
    TA_K_OUT_PY(TA_CDL3OUTSIDE)
    TA_K_OUT_PY(TA_CDL3STARSINSOUTH)
    TA_K_OUT_PY(TA_CDL3WHITESOLDIERS)

    TA_K_OUT_PY(TA_CDLADVANCEBLOCK)
    TA_K_OUT_PY(TA_CDLBELTHOLD)
    TA_K_OUT_PY(TA_CDLBREAKAWAY)
    TA_K_OUT_PY(TA_CDLCLOSINGMARUBOZU)
    TA_K_OUT_PY(TA_CDLCONCEALBABYSWALL)
    TA_K_OUT_PY(TA_CDLCOUNTERATTACK)

    TA_K_OUT_PY(TA_CDLDOJI)
    TA_K_OUT_PY(TA_CDLDOJISTAR)
    TA_K_OUT_PY(TA_CDLDRAGONFLYDOJI)
    TA_K_OUT_PY(TA_CDLENGULFING)

    TA_K_OUT_PY(TA_CDLGAPSIDESIDEWHITE)
    TA_K_OUT_PY(TA_CDLGRAVESTONEDOJI)
    TA_K_OUT_PY(TA_CDLHAMMER)
    TA_K_OUT_PY(TA_CDLHANGINGMAN)
    TA_K_OUT_PY(TA_CDLHARAMI)
    TA_K_OUT_PY(TA_CDLHARAMICROSS)
    TA_K_OUT_PY(TA_CDLHIGHWAVE)
    TA_K_OUT_PY(TA_CDLHIKKAKE)
    TA_K_OUT_PY(TA_CDLHIKKAKEMOD)
    TA_K_OUT_PY(TA_CDLHOMINGPIGEON)
    TA_K_OUT_PY(TA_CDLIDENTICAL3CROWS)
    TA_K_OUT_PY(TA_CDLINNECK)
    TA_K_OUT_PY(TA_CDLINVERTEDHAMMER)
    TA_K_OUT_PY(TA_CDLKICKING)
    TA_K_OUT_PY(TA_CDLKICKINGBYLENGTH)
    TA_K_OUT_PY(TA_CDLLADDERBOTTOM)
    TA_K_OUT_PY(TA_CDLLONGLEGGEDDOJI)
    TA_K_OUT_PY(TA_CDLLONGLINE)
    TA_K_OUT_PY(TA_CDLMARUBOZU)
    TA_K_OUT_PY(TA_CDLMATCHINGLOW)

    TA_K_OUT_PY(TA_CDLONNECK)
    TA_K_OUT_PY(TA_CDLPIERCING)
    TA_K_OUT_PY(TA_CDLRICKSHAWMAN)
    TA_K_OUT_PY(TA_CDLRISEFALL3METHODS)
    TA_K_OUT_PY(TA_CDLSEPARATINGLINES)
    TA_K_OUT_PY(TA_CDLSHOOTINGSTAR)
    TA_K_OUT_PY(TA_CDLSHORTLINE)
    TA_K_OUT_PY(TA_CDLSPINNINGTOP)
    TA_K_OUT_PY(TA_CDLSTALLEDPATTERN)
    TA_K_OUT_PY(TA_CDLSTICKSANDWICH)
    TA_K_OUT_PY(TA_CDLTAKURI)
    TA_K_OUT_PY(TA_CDLTASUKIGAP)
    TA_K_OUT_PY(TA_CDLTHRUSTING)
    TA_K_OUT_PY(TA_CDLTRISTAR)
    TA_K_OUT_PY(TA_CDLUNIQUE3RIVER)
    TA_K_OUT_PY(TA_CDLUPSIDEGAP2CROWS)
    TA_K_OUT_PY(TA_CDLXSIDEGAP3METHODS)

    TA_IN1_OUT_PY(TA_CEIL)
    TA_IN1_OUT_N_PY(TA_CMO, 14)
    TA_IN1_OUT_PY(TA_COS)
    TA_IN1_OUT_PY(TA_COSH)
    TA_IN1_OUT_N_PY(TA_DEMA, 30)
    TA_K_OUT_N_PY(TA_DX, 14)
    TA_IN1_OUT_N_PY(TA_EMA, 30)
    TA_IN1_OUT_PY(TA_EXP)
    TA_IN1_OUT_PY(TA_FLOOR)
    TA_IN1_OUT_PY(TA_HT_DCPERIOD)
    TA_IN1_OUT_PY(TA_HT_DCPHASE)

    m.def("TA_HT_PHASOR", py::overload_cast<>(TA_HT_PHASOR));
    m.def("TA_HT_PHASOR", py::overload_cast<const Indicator&>(TA_HT_PHASOR));

    m.def("TA_HT_SINE", py::overload_cast<>(TA_HT_SINE));
    m.def("TA_HT_SINE", py::overload_cast<const Indicator&>(TA_HT_SINE));

    TA_IN1_OUT_PY(TA_HT_TRENDLINE)

    m.def("TA_HT_TRENDMODE", py::overload_cast<>(TA_HT_TRENDMODE));
    m.def("TA_HT_TRENDMODE", py::overload_cast<const Indicator&>(TA_HT_TRENDMODE));

    TA_K_OUT_N_PY(TA_IMI, 14)
    TA_IN1_OUT_N_PY(TA_KAMA, 30)
    TA_IN1_OUT_N_PY(TA_LINEARREG_ANGLE, 14)
    TA_IN1_OUT_N_PY(TA_LINEARREG_INTERCEPT, 14)
    TA_IN1_OUT_N_PY(TA_LINEARREG_SLOPE, 14)
    TA_IN1_OUT_N_PY(TA_LINEARREG, 14)
    TA_IN1_OUT_PY(TA_LN)
    TA_IN1_OUT_PY(TA_LOG10)

    m.def("TA_MA", py::overload_cast<int, int>(TA_MA), py::arg("n") = 30, py::arg("matype") = 0);
    m.def("TA_MA", py::overload_cast<const Indicator&, int, int>(TA_MA), py::arg("data"),
          py::arg("n") = 30, py::arg("matype") = 0);

    m.def("TA_MACD", py::overload_cast<int, int, int>(TA_MACD), py::arg("fast_n") = 12,
          py::arg("slow_n") = 26, py::arg("signal_n") = 9);
    m.def("TA_MACD", py::overload_cast<const Indicator&, int, int, int>(TA_MACD), py::arg("data"),
          py::arg("fast_n") = 30, py::arg("slow_n") = 26, py::arg("signal_n") = 9);

    m.def("TA_MACDEXT", py::overload_cast<int, int, int, int, int, int>(TA_MACDEXT),
          py::arg("fast_n") = 12, py::arg("slow_n") = 26, py::arg("signal_n") = 9,
          py::arg("fast_matype") = 0, py::arg("slow_matype") = 0, py::arg("signal_matype") = 0);
    m.def("TA_MACDEXT",
          py::overload_cast<const Indicator&, int, int, int, int, int, int>(TA_MACDEXT),
          py::arg("data"), py::arg("fast_n") = 30, py::arg("slow_n") = 26, py::arg("signal_n") = 9,
          py::arg("fast_matype") = 0, py::arg("slow_matype") = 0, py::arg("signal_matype") = 0);

    m.def("TA_MACDFIX", py::overload_cast<int>(TA_MACDFIX), py::arg("n") = 9);
    m.def("TA_MACDFIX", py::overload_cast<const Indicator&, int>(TA_MACDFIX), py::arg("data"),
          py::arg("n") = 9);

    m.def("TA_MAMA", py::overload_cast<double, double>(TA_MAMA),
          py::arg("fast_limit") = 5.000000e-1, py::arg("slow_limit") = 5.000000e-2);
    m.def("TA_MAMA", py::overload_cast<const Indicator&, double, double>(TA_MAMA), py::arg("data"),
          py::arg("fast_limit") = 5.000000e-1, py::arg("slow_limit") = 5.000000e-2);

    TA_IN1_OUT_N_PY(TA_MAX, 30)

    m.def("TA_MAXINDEX", py::overload_cast<int>(TA_MAXINDEX), py::arg("n") = 30);
    m.def("TA_MAXINDEX", py::overload_cast<const Indicator&, int>(TA_MAXINDEX), py::arg("data"),
          py::arg("n") = 30);

    TA_K_OUT_PY(TA_MEDPRICE)
    TA_K_OUT_N_PY(TA_MFI, 14)
    TA_IN1_OUT_N_PY(TA_MIDPOINT, 14)
    TA_K_OUT_N_PY(TA_MIDPRICE, 14)
    TA_IN1_OUT_N_PY(TA_MIN, 30)

    m.def("TA_MININDEX", py::overload_cast<int>(TA_MININDEX), py::arg("n") = 30);
    m.def("TA_MININDEX", py::overload_cast<const Indicator&, int>(TA_MININDEX), py::arg("data"),
          py::arg("n") = 30);

    TA_IN1_OUT_N_PY(TA_MINMAX, 30)

    m.def("TA_MINMAXINDEX", py::overload_cast<int>(TA_MINMAXINDEX), py::arg("n") = 30);
    m.def("TA_MINMAXINDEX", py::overload_cast<const Indicator&, int>(TA_MINMAXINDEX),
          py::arg("data"), py::arg("n") = 30);

    TA_K_OUT_N_PY(TA_MINUS_DI, 14)
    TA_K_OUT_N_PY(TA_MINUS_DM, 14)
    TA_IN1_OUT_N_PY(TA_MOM, 10)
    TA_K_OUT_N_PY(TA_NATR, 14)
    TA_K_OUT_N_PY(TA_PLUS_DI, 14)
    TA_K_OUT_N_PY(TA_PLUS_DM, 14)
    TA_IN1_OUT_N_PY(TA_ROC, 10)
    TA_IN1_OUT_N_PY(TA_ROCP, 10)
    TA_IN1_OUT_N_PY(TA_ROCR, 10)
    TA_IN1_OUT_N_PY(TA_ROCR100, 10)
    TA_IN1_OUT_N_PY(TA_RSI, 14)
    TA_IN1_OUT_PY(TA_SIN)
    TA_IN1_OUT_PY(TA_SINH)
    TA_IN1_OUT_N_PY(TA_SMA, 30)
    TA_IN1_OUT_PY(TA_SQRT)
    TA_IN1_OUT_PY(TA_TAN)
    TA_IN1_OUT_PY(TA_TANH)
    TA_IN1_OUT_N_PY(TA_TEMA, 30)
    TA_K_OUT_PY(TA_TRANGE)
    TA_IN1_OUT_N_PY(TA_TRIMA, 30)
    TA_IN1_OUT_N_PY(TA_TRIX, 30)
    TA_IN1_OUT_N_PY(TA_TSF, 14)
    TA_K_OUT_PY(TA_TYPPRICE)
    TA_K_OUT_PY(TA_WCLPRICE)
    TA_K_OUT_N_PY(TA_WILLR, 14)
    TA_IN1_OUT_N_PY(TA_WMA, 30)
}

#endif /* HKU_ENABLE_TA_LIB */