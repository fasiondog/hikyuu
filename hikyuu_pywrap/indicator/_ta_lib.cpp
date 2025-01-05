/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator_talib/ta_crt.h>

#if HKU_ENABLE_TA_LIB

#include "../pybind_utils.h"

#define TA_IN1_OUT_PY(func, doc)                             \
    m.def(#func, py::overload_cast<>(func));                 \
    m.def(#func, py::overload_cast<const Indicator&>(func)); \
    m.def(#func, py::overload_cast<Indicator::value_t>(func), doc);

#define TA_IN1_OUT_N_PY(func, period, doc)                                        \
    m.def(#func, py::overload_cast<int>(func), py::arg("n") = period);            \
    m.def(#func, py::overload_cast<const IndParam&>(func));                       \
    m.def(#func, py::overload_cast<const Indicator&, const IndParam&>(func));     \
    m.def(#func, py::overload_cast<const Indicator&, const Indicator&>(func));    \
    m.def(#func, py::overload_cast<const Indicator&, int>(func), py::arg("data"), \
          py::arg("n") = period, doc);

#define TA_IN2_OUT_PY(func, description)                                            \
    m.def(#func, py::overload_cast<bool>(func), py::arg("fill_null") = true);       \
    m.def(#func, py::overload_cast<const Indicator&, const Indicator&, bool>(func), \
          py::arg("ind1"), py::arg("ind2"), py::arg("fill_null") = true,            \
          description "\n\n:param Indicator ind1: input1\n:param Indicator ind2: input2");

#define TA_IN2_OUT_N_PY(func, period, description)                                              \
    m.def(#func, py::overload_cast<int, bool>(func), py::arg("n") = period,                     \
          py::arg("fill_null") = true);                                                         \
    m.def(#func, py::overload_cast<const Indicator&, const Indicator&, int, bool>(func),        \
          py::arg("ind1"), py::arg("ind2"), py::arg("n") = period, py::arg("fill_null") = true, \
          description);

#define TA_K_OUT_PY(func, doc)               \
    m.def(#func, py::overload_cast<>(func)); \
    m.def(#func, py::overload_cast<const KData&>(func), py::arg("data"), doc);

#define TA_K_OUT_N_PY(func, period, doc)                                      \
    m.def(#func, py::overload_cast<int>(func), py::arg("n") = period);        \
    m.def(#func, py::overload_cast<const KData&, int>(func), py::arg("data"), \
          py::arg("n") = period, doc);

#define TA_K_OUT_P_D_PY(func, param, param_val, doc)                             \
    m.def(#func, py::overload_cast<double>(func), py::arg(#param) = param_val);  \
    m.def(#func, py::overload_cast<const KData&, double>(func), py::arg("data"), \
          py::arg(#param) = param_val, doc);

namespace py = pybind11;
using namespace hku;

void export_Indicator_ta_lib(py::module& m) {
    TA_K_OUT_N_PY(TA_ACCBANDS, 20, R"(TA_ACCBANDS - Acceleration Bands

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_PY(TA_ACOS, "TA_ACOS - Vector Trigonometric ACos")
    TA_K_OUT_PY(TA_AD, "TA_AD - Chaikin A/D Line")
    TA_IN2_OUT_PY(TA_ADD, "TA_ADD - Vector Arithmetic Add")

    m.def("TA_ADOSC", py::overload_cast<int, int>(TA_ADOSC), py::arg("fast_n") = 3,
          py::arg("slow_n") = 10);
    m.def("TA_ADOSC", py::overload_cast<const KData&, int, int>(TA_ADOSC), py::arg("data"),
          py::arg("fast_n") = 3, py::arg("slow_n") = 10, R"(TA_ADOSC - Chaikin A/D Oscillator

:param KData data: input KData
:param int fast_n: Number of period for the fast MA (From 2 to 100000)
:param int slow_n: Number of period for the slow MA (From 2 to 100000))");

    TA_K_OUT_N_PY(TA_ADX, 14, R"(TA_ADX - Average Directional Movement Index

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_N_PY(TA_ADXR, 14, R"(TA_ADXR - Average Directional Movement Index Rating

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    m.def("TA_APO", py::overload_cast<int, int, int>(TA_APO), py::arg("fast_n") = 12,
          py::arg("slow_n") = 26, py::arg("matype") = 0);
    m.def("TA_APO", py::overload_cast<const Indicator&, int, int, int>(TA_APO), py::arg("data"),
          py::arg("fast_n") = 12, py::arg("slow_n") = 26, py::arg("matype") = 0,
          R"(TA_APO - Absolute Price Oscillator

:param Indicator data: input data
:param int fast_n: Number of period for the fast MA (From 2 to 100000)
:param int slow_n: Number of period for the slow MA (From 2 to 100000)    
:param int matype: Type of Moving Average)");

    TA_K_OUT_N_PY(TA_AROON, 14, R"(TA_AROON - Aroon

:param KData data: input KData
:param int n: Number of period (From 2 to 100000)
:return: result(0) - Aroon down
         result(2) - Aroon up)")

    TA_K_OUT_N_PY(TA_AROONOSC, 14, R"(TA_AROONOSC - Aroon Oscillator

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_PY(TA_ASIN, "TA_ASIN - Vector Trigonometric ASin")
    TA_IN1_OUT_PY(TA_ATAN, "TA_ATAN - Vector Trigonometric ATan")

    TA_K_OUT_N_PY(TA_ATR, 14, R"(TA_ATR - Average True Range
    
:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_AVGDEV, 14, R"(TA_AVGDEV - Average Deviation

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_PY(TA_AVGPRICE, "TA_AVGPRICE - Average Price")

    m.def("TA_BBANDS", py::overload_cast<int, double, double, int>(TA_BBANDS), py::arg("n") = 5,
          py::arg("nbdevup") = 2., py::arg("nbdevdn") = 2., py::arg("matype") = 0);
    m.def("TA_BBANDS", py::overload_cast<const Indicator&, int, double, double, int>(TA_BBANDS),
          py::arg("data"), py::arg("n") = 5, py::arg("nbdevup") = 2., py::arg("nbdevdn") = 2.,
          py::arg("matype") = 0, R"(TA_BBANDS - Bollinger Bands
          
:param Indicator data: input data
:param int n: Number of periode (From 1 to 100000)
:param float nbdevup: Deviation multiplier for upper band
:param float nbdevdn: Deviation multiplier for lower band
:rtype: 具有三个结果集的 Indicator

    * result(0): Upper Band
    * result(1): Middle Band
    * result(2): Lower Band)");

    TA_IN2_OUT_N_PY(TA_BETA, 5, R"(TA_BETA - Beta

:param Indicator ind1: input1
:param Indicator ind2: input2
:param int n: Number of periode (From 1 to 100000))")

    TA_K_OUT_PY(TA_BOP, "TA_BOP - Balance Of Power")

    TA_K_OUT_N_PY(TA_CCI, 14, R"(TA_CCI - Commodity Channel Index

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_PY(TA_CDL2CROWS, "TA_CDL2CROWS - Two Crows")
    TA_K_OUT_PY(TA_CDL3BLACKCROWS, "TA_CDL3BLACKCROWS - Three Black Crows")
    TA_K_OUT_PY(TA_CDL3INSIDE, "TA_CDL3INSIDE - Three Inside Up/Down")
    TA_K_OUT_PY(TA_CDL3LINESTRIKE, "TA_CDL3LINESTRIKE - Three-Line Strike")
    TA_K_OUT_PY(TA_CDL3OUTSIDE, "TA_CDL3OUTSIDE - Three Outside Up/Down")
    TA_K_OUT_PY(TA_CDL3STARSINSOUTH, "TA_CDL3STARSINSOUTH - Three Stars In The South")
    TA_K_OUT_PY(TA_CDL3WHITESOLDIERS, "TA_CDL3WHITESOLDIERS - Three Advancing White Soldiers")
    TA_K_OUT_P_D_PY(TA_CDLABANDONEDBABY, penetration, 3.000000e-1,
                    R"(TA_CDLABANDONEDBABY - Abandoned Baby

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_PY(TA_CDLADVANCEBLOCK, "TA_CDLADVANCEBLOCK - Advance Block")
    TA_K_OUT_PY(TA_CDLBELTHOLD, "TA_CDLBELTHOLD - Belt-hold")
    TA_K_OUT_PY(TA_CDLBREAKAWAY, "TA_CDLBREAKAWAY - Breakaway")
    TA_K_OUT_PY(TA_CDLCLOSINGMARUBOZU, "TA_CDLCLOSINGMARUBOZU - Closing Marubozu")
    TA_K_OUT_PY(TA_CDLCONCEALBABYSWALL, "TA_CDLCONCEALBABYSWALL - Concealing Baby Swallow")
    TA_K_OUT_PY(TA_CDLCOUNTERATTACK, "TA_CDLCOUNTERATTACK - Counterattack")
    TA_K_OUT_P_D_PY(TA_CDLDARKCLOUDCOVER, penetration, 5.000000e-1,
                    R"(TA_CDLDARKCLOUDCOVER - Dark Cloud Cover

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_PY(TA_CDLDOJI, "TA_CDLDOJI - Doji")
    TA_K_OUT_PY(TA_CDLDOJISTAR, "TA_CDLDOJISTAR - Doji Star")
    TA_K_OUT_PY(TA_CDLDRAGONFLYDOJI, "TA_CDLDRAGONFLYDOJI - Dragonfly Doji")
    TA_K_OUT_PY(TA_CDLENGULFING, "TA_CDLENGULFING - Engulfing Pattern")
    TA_K_OUT_P_D_PY(TA_CDLEVENINGDOJISTAR, penetration, 3.000000e-1,
                    R"(TA_CDLEVENINGDOJISTAR - Evening Doji Star

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_P_D_PY(TA_CDLEVENINGSTAR, penetration, 3.000000e-1,
                    R"(TA_CDLEVENINGSTAR - Evening Star
            
:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_PY(TA_CDLGAPSIDESIDEWHITE,
                "TA_CDLGAPSIDESIDEWHITE - Up/Down-gap side-by-side white lines")
    TA_K_OUT_PY(TA_CDLGRAVESTONEDOJI, "TA_CDLGRAVESTONEDOJI - Gravestone Doji")
    TA_K_OUT_PY(TA_CDLHAMMER, "TA_CDLHAMMER - Hammer")
    TA_K_OUT_PY(TA_CDLHANGINGMAN, "TA_CDLHANGINGMAN - Hanging Man")
    TA_K_OUT_PY(TA_CDLHARAMI, "TA_CDLHARAMI - Harami Pattern")
    TA_K_OUT_PY(TA_CDLHARAMICROSS, "TA_CDLHARAMICROSS - Harami Cross Pattern")
    TA_K_OUT_PY(TA_CDLHIGHWAVE, "TA_CDLHIGHWAVE - High-Wave Candle")
    TA_K_OUT_PY(TA_CDLHIKKAKE, "TA_CDLHIKKAKE - Hikkake Pattern")
    TA_K_OUT_PY(TA_CDLHIKKAKEMOD, "TA_CDLHIKKAKEMOD - Modified Hikkake Pattern")
    TA_K_OUT_PY(TA_CDLHOMINGPIGEON, "TA_CDLHOMINGPIGEON - Homing Pigeon")
    TA_K_OUT_PY(TA_CDLIDENTICAL3CROWS, "TA_CDLIDENTICAL3CROWS - Identical Three Crows")
    TA_K_OUT_PY(TA_CDLINNECK, "TA_CDLINNECK - In-Neck Pattern")
    TA_K_OUT_PY(TA_CDLINVERTEDHAMMER, "TA_CDLINVERTEDHAMMER - Inverted Hammer")
    TA_K_OUT_PY(TA_CDLKICKING, "TA_CDLKICKING - Kicking")
    TA_K_OUT_PY(TA_CDLKICKINGBYLENGTH,
                "TA_CDLKICKINGBYLENGTH - Kicking - bull/bear determined by the longer marubozu")
    TA_K_OUT_PY(TA_CDLLADDERBOTTOM, "TA_CDLLADDERBOTTOM - Ladder Bottom")
    TA_K_OUT_PY(TA_CDLLONGLEGGEDDOJI, "TA_CDLLONGLEGGEDDOJI - Long Legged Doji")
    TA_K_OUT_PY(TA_CDLLONGLINE, "TA_CDLLONGLINE - Long Line Candle")
    TA_K_OUT_PY(TA_CDLMARUBOZU, "TA_CDLMARUBOZU - Marubozu")
    TA_K_OUT_PY(TA_CDLMATCHINGLOW, "TA_CDLMATCHINGLOW - Matching Low")
    TA_K_OUT_P_D_PY(TA_CDLMATHOLD, penetration, 5.000000e-1, R"(TA_CDLMATHOLD - Mat Hold

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_P_D_PY(TA_CDLMORNINGDOJISTAR, penetration, 3.000000e-1,
                    R"(TA_CDLMORNINGDOJISTAR - Morning Doji Star

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_P_D_PY(TA_CDLMORNINGSTAR, penetration, 3.000000e-1, R"(TA_CDLMORNINGSTAR - Morning Star

:param KData data: input KData
:param float penetration: Percentage of penetration of a candle within another candle (>=0))")

    TA_K_OUT_PY(TA_CDLONNECK, "TA_CDLONNECK - On-Neck Pattern")
    TA_K_OUT_PY(TA_CDLPIERCING, "TA_CDLPIERCING - Piercing Pattern")
    TA_K_OUT_PY(TA_CDLRICKSHAWMAN, "TA_CDLRICKSHAWMAN - Rickshaw Man")
    TA_K_OUT_PY(TA_CDLRISEFALL3METHODS, "TA_CDLRISEFALL3METHODS - Rising/Falling Three Methods")
    TA_K_OUT_PY(TA_CDLSEPARATINGLINES, "TA_CDLSEPARATINGLINES - Separating Lines")
    TA_K_OUT_PY(TA_CDLSHOOTINGSTAR, "TA_CDLSHOOTINGSTAR - Shooting Star")
    TA_K_OUT_PY(TA_CDLSHORTLINE, "TA_CDLSHORTLINE - Short Line Candle")
    TA_K_OUT_PY(TA_CDLSPINNINGTOP, "TA_CDLSPINNINGTOP - Spinning Top")
    TA_K_OUT_PY(TA_CDLSTALLEDPATTERN, "TA_CDLSTALLEDPATTERN - Stalled Pattern")
    TA_K_OUT_PY(TA_CDLSTICKSANDWICH, "TA_CDLSTICKSANDWICH - Stick Sandwich")
    TA_K_OUT_PY(TA_CDLTAKURI, "TA_CDLTAKURI - Takuri (Dragonfly Doji with very long lower shadow)")
    TA_K_OUT_PY(TA_CDLTASUKIGAP, "TA_CDLTASUKIGAP - Tasuki Gap")
    TA_K_OUT_PY(TA_CDLTHRUSTING, "TA_CDLTHRUSTING - Thrusting Pattern")
    TA_K_OUT_PY(TA_CDLTRISTAR, "TA_CDLTRISTAR - Tristar Pattern")
    TA_K_OUT_PY(TA_CDLUNIQUE3RIVER, "TA_CDLUNIQUE3RIVER - Unique 3 River")
    TA_K_OUT_PY(TA_CDLUPSIDEGAP2CROWS, "TA_CDLUPSIDEGAP2CROWS - Upside Gap Two Crows")
    TA_K_OUT_PY(TA_CDLXSIDEGAP3METHODS,
                "TA_CDLXSIDEGAP3METHODS - Upside/Downside Gap Three Methods")

    TA_IN1_OUT_PY(TA_CEIL, "TA_CEIL - Vector Ceil")

    TA_IN1_OUT_N_PY(TA_CMO, 14, R"(TA_CMO - Chande Momentum Oscillator

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN2_OUT_N_PY(TA_CORREL, 30, R"(TA_CORREL - Pearson's Correlation Coefficient (r)
    
:param Indicator ind1: input1
:param Indicator ind2: input2
:param int n: Number of periode (From 1 to 100000))")

    TA_IN1_OUT_PY(TA_COS, "TA_COS - Vector Trigonometric Cos")
    TA_IN1_OUT_PY(TA_COSH, "TA_COSH - Vector Trigonometric Cosh")
    TA_IN1_OUT_N_PY(TA_DEMA, 30, R"(TA_DEMA - Double Exponential Moving Average

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN2_OUT_PY(TA_DIV, "TA_DIV - Vector Arithmetic Div")
    TA_K_OUT_N_PY(TA_DX, 14, R"(TA_DX - Directional Movement Index

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_EMA, 30, R"(TA_EMA - Exponential Moving Average

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_PY(TA_EXP, "TA_EXP - Vector Arithmetic Exp")
    TA_IN1_OUT_PY(TA_FLOOR, "TA_FLOOR - Vector Floor")
    TA_IN1_OUT_PY(TA_HT_DCPERIOD, "TA_HT_DCPERIOD - Hilbert Transform - Dominant Cycle Period")
    TA_IN1_OUT_PY(TA_HT_DCPHASE, "TA_HT_DCPHASE - Hilbert Transform - Dominant Cycle Phase")
    TA_IN1_OUT_PY(TA_HT_PHASOR, R"(TA_HT_PHASOR - Hilbert Transform - Phasor Components

:return: result(0) - outInPhase
         result(1) - outQuadrature)")

    TA_IN1_OUT_PY(TA_HT_SINE, R"(TA_HT_SINE - Hilbert Transform - SineWave
:return: result(0) - outSine
         result(1) - outLeadSine)")

    TA_IN1_OUT_PY(TA_HT_TRENDLINE, "TA_HT_TRENDLINE - Hilbert Transform - Instantaneous Trendline")
    TA_IN1_OUT_PY(TA_HT_TRENDMODE, "TA_HT_TRENDMODE - Hilbert Transform - Trend vs Cycle Mode")
    TA_K_OUT_N_PY(TA_IMI, 14, R"(TA_IMI - Intraday Momentum Index

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_KAMA, 30, R"(TA_KAMA - Kaufman Adaptive Moving Average

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_LINEARREG_ANGLE, 14, R"(TA_LINEARREG_ANGLE - Linear Regression Angle

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_LINEARREG_INTERCEPT, 14,
                    R"(TA_LINEARREG_INTERCEPT - Linear Regression Intercept

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_LINEARREG_SLOPE, 14, R"(TA_LINEARREG_SLOPE - Linear Regression Slope

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_LINEARREG, 14, R"(TA_LINEARREG - Linear Regression

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_PY(TA_LN, "TA_LN - Vector Log Natural")
    TA_IN1_OUT_PY(TA_LOG10, "TA_LOG10 - Vector Log10")

    m.def("TA_MA", py::overload_cast<int, int>(TA_MA), py::arg("n") = 30, py::arg("matype") = 0);
    m.def("TA_MA", py::overload_cast<const Indicator&, int, int>(TA_MA), py::arg("data"),
          py::arg("n") = 30, py::arg("matype") = 0, R"(TA_MA - Moving average

:param Indicator data: input data
:param int n: Number of periode (From 1 to 100000)
:param int matype: Type of Moving Average)");

    m.def("TA_MACD", py::overload_cast<int, int, int>(TA_MACD), py::arg("fast_n") = 12,
          py::arg("slow_n") = 26, py::arg("signal_n") = 9);
    m.def("TA_MACD", py::overload_cast<const Indicator&, int, int, int>(TA_MACD), py::arg("data"),
          py::arg("fast_n") = 30, py::arg("slow_n") = 26, py::arg("signal_n") = 9,
          R"(TA_MACD - Moving Average Convergence/Divergence

:param Indicator data: input data
:param int fast_n: Number of periode for fast MA (From 2 to 100000)
:param int slow_n: Number of periode for slow MA (From 2 to 100000)
:param int signal_n: Smoothing for the signal line (nb of period) (From 1 to 100000))");

    m.def("TA_MACDEXT", py::overload_cast<int, int, int, int, int, int>(TA_MACDEXT),
          py::arg("fast_n") = 12, py::arg("slow_n") = 26, py::arg("signal_n") = 9,
          py::arg("fast_matype") = 0, py::arg("slow_matype") = 0, py::arg("signal_matype") = 0);
    m.def("TA_MACDEXT",
          py::overload_cast<const Indicator&, int, int, int, int, int, int>(TA_MACDEXT),
          py::arg("data"), py::arg("fast_n") = 30, py::arg("slow_n") = 26, py::arg("signal_n") = 9,
          py::arg("fast_matype") = 0, py::arg("slow_matype") = 0, py::arg("signal_matype") = 0,
          R"(TA_MACDEXT - MACD with controllable MA type

:param Indicator data: input data
:param int fast_n: Number of periode for fast MA (From 2 to 100000)
:param int slow_n: Number of periode for slow MA (From 2 to 100000)
:param int signal_n: Smoothing for the signal line (nb of period) (From 1 to 100000)
:param int fast_matype: Type of Moving Average for fast MA
:param int slow_matype: Type of Moving Average for slow MA
:param int signal_matype: Type of Moving Average for signal line)");

    TA_IN1_OUT_N_PY(TA_MACDFIX, 9, R"(TA_MACDFIX - Moving Average Convergence/Divergence Fix 12/26

:param Indicator data: input data
:param int n: Smoothing for the signal line (nb of period) (From 1 to 100000)
:return: result(0) - outMACD
         result(1) - outMACDSignal
         result(2) - outMACDHist)")

    m.def("TA_MAMA", py::overload_cast<double, double>(TA_MAMA),
          py::arg("fast_limit") = 5.000000e-1, py::arg("slow_limit") = 5.000000e-2);
    m.def("TA_MAMA", py::overload_cast<const Indicator&, double, double>(TA_MAMA), py::arg("data"),
          py::arg("fast_limit") = 5.000000e-1, py::arg("slow_limit") = 5.000000e-2,
          R"(TA_MAMA - MESA Adaptive Moving Average

:param Indicator data: input data
:param float fast_limit: Fast limit (From 0.01 to 0.99)
:param float slow_limit: Slow limit (From 0.01 to 0.99))");

    m.def("TA_MAVP", py::overload_cast<const Indicator&, int, int, int, bool>(TA_MAVP),
          py::arg("ref_ind"), py::arg("min_n") = 2, py::arg("max_n") = 30, py::arg("matype") = 0,
          py::arg("fill_null") = true);
    m.def("TA_MAVP",
          py::overload_cast<const Indicator&, const Indicator&, int, int, int, bool>(TA_MAVP),
          py::arg("ind1"), py::arg("ind2"), py::arg("min_n") = 2, py::arg("max_n") = 30,
          py::arg("fill_null") = true, py::arg("matype") = 0,
          R"(TA_MAVP - Moving average with variable period

:param Indicator ind1: input1
:param Indicator ind2: input2
:param int min_n: Value less than minimum will be changed to Minimum period (From 2 to 100000)
:param int max_n: Value higher than maximum will be changed to Maximum period (From 2 to 100000)
:param int matype: Type of Moving Average
:param bool fill_null: 日期对齐时，缺失日期数据填充nan值)");

    TA_IN1_OUT_N_PY(TA_MAX, 30, R"(TA_MAX - Highest value over a specified period

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_MAXINDEX, 30, R"(TA_MAXINDEX - Index of highest value over a specified period

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_PY(TA_MEDPRICE, "TA_MEDPRICE - Median Price")
    TA_K_OUT_N_PY(TA_MFI, 14, R"(TA_MFI - Money Flow Index

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_MIDPOINT, 14, R"(TA_MIDPOINT - MidPoint over period

:param Indicator data: input
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_N_PY(TA_MIDPRICE, 14, R"(TA_MIDPRICE - Midpoint Price over period

:param KData data: input KData
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_MIN, 30, R"(TA_MIN - Lowest value over a specified period

:param Indicator data: input
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_MININDEX, 30, R"(TA_MININDEX - Index of lowest value over a specified period

:param Indicator data: input
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_MINMAX, 30, R"(TA_MINMAX - Lowest and highest values over a specified period

:param Indicator data: input
:param int n: Number of period (From 2 to 100000)
:return: result(0) - outMin
         result(1) - outMax)")

    TA_IN1_OUT_N_PY(TA_MINMAXINDEX, 30,
                    R"(TA_MINMAXINDEX - Indexes of lowest and highest values over a specified period
    
:param Indicator data: input
:param int n: Number of period (From 2 to 100000)
:return: result(0) - outMinIdx
         result(1) - outMaxIdx)")

    TA_K_OUT_N_PY(TA_MINUS_DI, 14, R"(A_MINUS_DI - Minus Directional Indicator

:param KData data: input KData
:param int n: Number of period (From 1 to 100000))")

    TA_K_OUT_N_PY(TA_MINUS_DM, 14, R"(TA_MINUS_DM - Minus Directional Movement

:param KData data: input KData
:param int n: Number of period (From 1 to 100000))")

    m.def("TA_PPO", py::overload_cast<int, int, int>(TA_PPO), py::arg("fast_n") = 12,
          py::arg("slow_n") = 26, py::arg("matype") = 0);
    m.def("TA_PPO", py::overload_cast<const Indicator&, int, int, int>(TA_PPO), py::arg("data"),
          py::arg("fast_n") = 12, py::arg("slow_n") = 26, py::arg("matype") = 0,
          R"(TA_PPO - Percentage Price Oscillator

:param Indicator data: input data
:param int fast_n: Number of periode for fast MA (From 2 to 100000)
:param int slow_n: Number of periode for slow MA (From 2 to 100000)
:param int matype: Type of Moving Average)");

    TA_IN1_OUT_N_PY(TA_MOM, 10, R"(TA_MOM - Momentum
    
:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN2_OUT_PY(TA_MULT, "TA_MULT - Vector Arithmetic Mult")
    TA_K_OUT_N_PY(TA_NATR, 14, R"(TA_NATR - Normalized Average True Range

:param KData data: input KData
:param int n: Number of period (From 1 to 100000))")

    TA_K_OUT_PY(TA_OBV, "TA_OBV - On Balance Volume")
    TA_K_OUT_N_PY(TA_PLUS_DI, 14, R"(TA_PLUS_DI - Plus Directional Indicator

:param KData data: input KData
:param int n: Number of period (From 1 to 100000))")

    TA_K_OUT_N_PY(TA_PLUS_DM, 14, R"(TA_PLUS_DM - Plus Directional Movement

:param KData data: input KData
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_ROC, 10, R"(TA_ROC - Rate of change : ((price/prevPrice)-1)*100
    
:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_ROCP, 10, R"(TA_ROCP - Rate of change Percentage: (price-prevPrice)/prevPrice

:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_ROCR, 10, R"(TA_ROCR - Rate of change ratio: (price/prevPrice)

:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_ROCR100, 10,
                    R"(TA_ROCR100 - Rate of change ratio 100 scale: (price/prevPrice)*100

:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_RSI, 14, R"(TA_RSI - Relative Strength Index

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    m.def("TA_SAR", py::overload_cast<double, double>(TA_SAR), py::arg("acceleration") = 0.02,
          py::arg("maximum") = 0.2);
    m.def("TA_SAR", py::overload_cast<const KData&, double, double>(TA_SAR), py::arg("data"),
          py::arg("acceleration") = 0.02, py::arg("maximum") = 0.2, R"(TA_SAR - Parabolic SAR

:param KData data: input KData object
:param float acceleration: Acceleration Factor used up to the Maximum value (>= 0.0)
:param float maximum: Acceleration Factor Maximum value (>= 0.0))");

    m.def(
      "TA_SAREXT",
      py::overload_cast<double, double, double, double, double, double, double, double>(TA_SAREXT),
      py::arg("startvalue") = 0.0, py::arg("offsetonreverse") = 0.0,
      py::arg("accelerationinitlong") = 0.02, py::arg("accelerationlong") = 0.02,
      py::arg("accelerationmaxlong") = 0.2, py::arg("accelerationinitshort") = 0.02,
      py::arg("accelerationshort") = 0.02, py::arg("accelerationmaxshort") = 0.2);
    m.def("TA_SAREXT",
          py::overload_cast<const KData&, double, double, double, double, double, double, double,
                            double>(TA_SAREXT),
          py::arg("data"), py::arg("startvalue") = 0.0, py::arg("offsetonreverse") = 0.0,
          py::arg("accelerationinitlong") = 0.02, py::arg("accelerationlong") = 0.02,
          py::arg("accelerationmaxlong") = 0.2, py::arg("accelerationinitshort") = 0.02,
          py::arg("accelerationshort") = 0.02, py::arg("accelerationmaxshort") = 0.2,
          R"(TA_SAREXT - Parabolic SAR - Extended

:param KData data: input KData object
:param float startvalue: Start value and direction. 0 for Auto, >0 for Long, <0 for Short
:param float offsetonreverse: Percent offset added/removed to initial stop on short/long reversal (>= 0.0)
:param float accelerationinitlong: Acceleration Factor initial value for the Long direction (>= 0.0)
:param float accelerationlong: Acceleration Factor for the Long direction (>= 0.0)
:param float accelerationmaxlong: Acceleration Factor maximum value for the Long direction (>= 0.0)
:param float accelerationinitshort: Acceleration Factor initial value for the Short direction (>= 0.0)
:param float accelerationshort: Acceleration Factor for the Short direction (>= 0.0)
:param float accelerationmaxshort: Acceleration Factor maximum value for the Short direction(>= 0.0))");

    TA_IN1_OUT_PY(TA_SIN, "TA_SIN - Vector Trigonometric Sin")
    TA_IN1_OUT_PY(TA_SINH, "TA_SINH - Vector Trigonometric Sinh")

    TA_IN1_OUT_N_PY(TA_SMA, 30, R"(TA_SMA - Simple Moving Average
    
:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_PY(TA_SQRT, "TA_SQRT - Vector Square Root")

    m.def("TA_STDDEV", py::overload_cast<int, double>(TA_STDDEV), py::arg("n") = 5,
          py::arg("nbdev") = 1.0);
    m.def("TA_STDDEV", py::overload_cast<const Indicator&, int, double>(TA_STDDEV), py::arg("data"),
          py::arg("n") = 5, py::arg("nbdev") = 1.0, R"(TA_STDDEV - Standard Deviation

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000)
:param float nbdev: Nb of deviations)");

    m.def("TA_STOCH", py::overload_cast<int, int, int, int, int>(TA_STOCH), py::arg("fastk_n") = 5,
          py::arg("slowk_n") = 3, py::arg("slowk_matype") = 0, py::arg("slowd_n") = 3,
          py::arg("slowd_matype") = 0);
    m.def("TA_STOCH", py::overload_cast<const KData&, int, int, int, int, int>(TA_STOCH),
          py::arg("data"), py::arg("fastk_n") = 5, py::arg("slowk_n") = 3,
          py::arg("slowk_matype") = 0, py::arg("slowd_n") = 3, py::arg("slowd_matype") = 0,
          R"(TA_STOCH - Stochastic
          
:parma KData data: KData object
:param int fastk_n: Time period for building the Fast-K line (From 1 to 100000)
:param int slowk_n: Smoothing for making the Slow-K line. Usually set to 3 (From 1 to 100000)
:param int slowk_matype: Type of Moving Average for Slow K (From 0 to 8)
:param int slowd_n: Smoothing for making the Slow-D line (From 1 to 100000)
:param int slowd_matype: Type of Moving Average for Slow D (From 0 to 8))  
:return: result0 - slow K 
         result1 - slow D)");

    m.def("TA_STOCHF", py::overload_cast<int, int, int>(TA_STOCHF), py::arg("fastk_n") = 5,
          py::arg("fastd_n") = 3, py::arg("fastd_matype") = 0);
    m.def("TA_STOCHF", py::overload_cast<const KData&, int, int, int>(TA_STOCHF), py::arg("data"),
          py::arg("fastk_n") = 5, py::arg("fastd_n") = 3, py::arg("fastd_matype") = 0,
          R"(TA_STOCHF - Stochastic Fast

:param KData data: input KData object
:param int fastk_n: Time period for building the Fast-K line (From 1 to 100000)
:param int fastd_n: Smoothing for making the Fast-D line. Usually set to 3 (From 1 to 100000)
:param int fastd_matype: Type of Moving Average for Fast D (From 0 to 8))
:return: result0 - fast K 
         result1 - fast D)");

    m.def("TA_STOCHRSI", py::overload_cast<int, int, int, int>(TA_STOCHRSI), py::arg("n") = 14,
          py::arg("fastk_n") = 5, py::arg("fastd_n") = 3, py::arg("matype") = 0);
    m.def("TA_STOCHRSI", py::overload_cast<const Indicator&, int, int, int, int>(TA_STOCHRSI),
          py::arg("data"), py::arg("n") = 14, py::arg("fastk_n") = 5, py::arg("fastd_n") = 3,
          py::arg("matype") = 0, R"(TA_STOCHRSI - Stochastic Relative Strength Index

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000)
:param int fastk_n: Time period for building the Fast-K line (From 1 to 100000)
:param int fastd_n: Smoothing for making the Fast-D line. Usually set to 3 (From 1 to 100000)
:param int matype: Type of Moving Average for Fast D (From 0 to 8))
:return: result0 - fast K 
         result1 - fast D)");

    TA_IN2_OUT_PY(TA_SUB, "TA_SUB - Vector Arithmetic Subtraction")
    TA_IN1_OUT_N_PY(TA_SUM, 30, R"(TA_SUM - Summation

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    m.def("TA_T3", py::overload_cast<int, double>(TA_T3), py::arg("n") = 5,
          py::arg("vfactor") = 7.000000e-1);
    m.def("TA_T3", py::overload_cast<const Indicator&, int, double>(TA_T3), py::arg("data"),
          py::arg("n") = 5, py::arg("vfactor") = 7.000000e-1,
          R"(TA_T3 - Triple Exponential Moving Average (T3)

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000)
:param float vfactor: Volume Factor (From 0 to 1))");

    TA_IN1_OUT_PY(TA_TAN, "TA_TAN - Vector Trigonometric Tan")
    TA_IN1_OUT_PY(TA_TANH, "TA_TANH - Vector Trigonometric Tanh")
    TA_IN1_OUT_N_PY(TA_TEMA, 30, R"(TA_TEMA - Triple Exponential Moving Average

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_PY(TA_TRANGE, "TA_TRANGE - True Range")
    TA_IN1_OUT_N_PY(TA_TRIMA, 30, R"(TA_TRIMA - Triangular Moving Average

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_TRIX, 30, R"(TA_TRIX - 1-day Rate-Of-Change (ROC) of a Triple Smooth EMA

:param Indicator data: input data
:param int n: Number of period (From 1 to 100000))")

    TA_IN1_OUT_N_PY(TA_TSF, 14, R"(TA_TSF - Time Series Forecast

:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")

    TA_K_OUT_PY(TA_TYPPRICE, "TA_TYPPRICE - Typical Price")

    m.def("TA_ULTOSC", py::overload_cast<int, int, int>(TA_ULTOSC), py::arg("n1") = 7,
          py::arg("n2") = 14, py::arg("n3") = 28);
    m.def("TA_ULTOSC", py::overload_cast<const KData&, int, int, int>(TA_ULTOSC), py::arg("data"),
          py::arg("n1") = 7, py::arg("n2") = 14, py::arg("n3") = 28,
          R"(TA_ULTOSC - Ultimate Oscillator

:param KData data: input KData object
:param int n1: Number of bars for 1st period (From 1 to 100000)
:param int n2: Number of bars fro 2nd period (From 1 to 100000)
:param int n3: Number of bars for 3rd period (From 1 to 100000))");

    m.def("TA_VAR", py::overload_cast<int, double>(TA_VAR), py::arg("n") = 5,
          py::arg("nbdev") = 1.0);
    m.def("TA_VAR", py::overload_cast<const Indicator&, int, double>(TA_VAR), py::arg("data"),
          py::arg("n") = 5, py::arg("nbdev") = 1.0, R"(TA_VAR - Variance

:param Indicator data: input data
:param int n: Number of period (From 1 to 100000)
:param float nbdev: Nb of deviations)");

    TA_K_OUT_PY(TA_WCLPRICE, "TA_WCLPRICE - Weighted Close Price")
    TA_K_OUT_N_PY(TA_WILLR, 14, R"(TA_WILLR - Williams' %R
    
:param KData data: input KData object
:param int n: Number of period (From 2 to 100000))")

    TA_IN1_OUT_N_PY(TA_WMA, 30, R"(TA_WMA - Weighted Moving Average
    
:param Indicator data: input data
:param int n: Number of period (From 2 to 100000))")
}

#endif /* HKU_ENABLE_TA_LIB */