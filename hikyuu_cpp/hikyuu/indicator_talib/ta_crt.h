/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/config.h"
#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/indicator/crt/CVAL.h"

#if HKU_ENABLE_TA_LIB

#define TA_IN1_OUT_CRT(func)                                \
    Indicator HKU_API func();                               \
    inline Indicator func(const Indicator& ind) {           \
        return func()(ind);                                 \
    }                                                       \
    inline Indicator HKU_API func(Indicator::value_t val) { \
        return func(CVAL(val));                             \
    }

#define TA_IN1_OUT_N_CRT(func, period)                                         \
    Indicator HKU_API func(int n = period);                                    \
    Indicator HKU_API func(const IndParam& n);                                 \
    inline Indicator func(const Indicator& ind, int n = period) {              \
        return func(n)(ind);                                                   \
    }                                                                          \
    inline Indicator HKU_API func(const Indicator& data, const IndParam& n) {  \
        return func(n)(data);                                                  \
    }                                                                          \
    inline Indicator HKU_API func(const Indicator& data, const Indicator& n) { \
        return func(IndParam(n))(data);                                        \
    }

#define TA_K_OUT_CRT(func)    \
    Indicator HKU_API func(); \
    Indicator HKU_API func(const KData& k);

#define TA_K_OUT_N_CRT(func, period)        \
    Indicator HKU_API func(int n = period); \
    Indicator HKU_API func(const KData& k, int n = period);

namespace hku {

TA_K_OUT_N_CRT(TA_ACCBANDS, 20)
TA_IN1_OUT_CRT(TA_ACOS)
TA_K_OUT_CRT(TA_AD)

Indicator HKU_API TA_ADOSC(int fast_n = 3, int slow_n = 10);
Indicator HKU_API TA_ADOSC(const KData& k, int fast_n = 3, int slow_n = 10);

TA_K_OUT_N_CRT(TA_ADX, 14)
TA_K_OUT_N_CRT(TA_ADXR, 14)

Indicator HKU_API TA_APO(int fast_n = 12, int slow_n = 26, int matype = 0);
inline Indicator TA_APO(const Indicator& ind, int fast_n = 12, int slow_n = 26, int matype = 0) {
    return TA_APO(fast_n, slow_n, matype)(ind);
}

TA_K_OUT_N_CRT(TA_AROON, 14)
TA_K_OUT_N_CRT(TA_AROONOSC, 14)

TA_IN1_OUT_CRT(TA_ASIN)
TA_IN1_OUT_CRT(TA_ATAN)
TA_K_OUT_N_CRT(TA_ATR, 14)

TA_IN1_OUT_N_CRT(TA_AVGDEV, 14)
TA_K_OUT_CRT(TA_AVGPRICE)

Indicator HKU_API TA_AVGPRICE();
Indicator HKU_API TA_AVGPRICE(const KData& k);

Indicator HKU_API TA_BBANDS(int n = 5, double nbdevup = 2., double nbdevdn = 2., int matype = 0);
inline Indicator TA_BBANDS(const Indicator& ind, int n = 5, double nbdevup = 2.,
                           double nbdevdn = 2., int matype = 0) {
    return TA_BBANDS(n, nbdevup, nbdevdn, matype)(ind);
}

TA_K_OUT_CRT(TA_BOP)
TA_K_OUT_N_CRT(TA_CCI, 14)
TA_K_OUT_CRT(TA_CDL2CROWS)
TA_K_OUT_CRT(TA_CDL3BLACKCROWS)
TA_K_OUT_CRT(TA_CDL3INSIDE)
TA_K_OUT_CRT(TA_CDL3LINESTRIKE)
TA_K_OUT_CRT(TA_CDL3OUTSIDE)
TA_K_OUT_CRT(TA_CDL3STARSINSOUTH)
TA_K_OUT_CRT(TA_CDL3WHITESOLDIERS)
TA_K_OUT_CRT(TA_CDLADVANCEBLOCK)
TA_K_OUT_CRT(TA_CDLBELTHOLD)
TA_K_OUT_CRT(TA_CDLBREAKAWAY)
TA_K_OUT_CRT(TA_CDLCLOSINGMARUBOZU)
TA_K_OUT_CRT(TA_CDLCONCEALBABYSWALL)
TA_K_OUT_CRT(TA_CDLCOUNTERATTACK)

TA_K_OUT_CRT(TA_CDLDOJI)
TA_K_OUT_CRT(TA_CDLDOJISTAR)
TA_K_OUT_CRT(TA_CDLDRAGONFLYDOJI)
TA_K_OUT_CRT(TA_CDLENGULFING)

TA_K_OUT_CRT(TA_CDLGAPSIDESIDEWHITE)
TA_K_OUT_CRT(TA_CDLGRAVESTONEDOJI)
TA_K_OUT_CRT(TA_CDLHAMMER)
TA_K_OUT_CRT(TA_CDLHANGINGMAN)
TA_K_OUT_CRT(TA_CDLHARAMI)
TA_K_OUT_CRT(TA_CDLHARAMICROSS)
TA_K_OUT_CRT(TA_CDLHIGHWAVE)
TA_K_OUT_CRT(TA_CDLHIKKAKE)
TA_K_OUT_CRT(TA_CDLHIKKAKEMOD)
TA_K_OUT_CRT(TA_CDLHOMINGPIGEON)
TA_K_OUT_CRT(TA_CDLIDENTICAL3CROWS)
TA_K_OUT_CRT(TA_CDLINNECK)
TA_K_OUT_CRT(TA_CDLINVERTEDHAMMER)
TA_K_OUT_CRT(TA_CDLKICKING)
TA_K_OUT_CRT(TA_CDLKICKINGBYLENGTH)
TA_K_OUT_CRT(TA_CDLLADDERBOTTOM)
TA_K_OUT_CRT(TA_CDLLONGLEGGEDDOJI)
TA_K_OUT_CRT(TA_CDLLONGLINE)
TA_K_OUT_CRT(TA_CDLMARUBOZU)
TA_K_OUT_CRT(TA_CDLMATCHINGLOW)

TA_K_OUT_CRT(TA_CDLONNECK)
TA_K_OUT_CRT(TA_CDLPIERCING)
TA_K_OUT_CRT(TA_CDLRICKSHAWMAN)
TA_K_OUT_CRT(TA_CDLRISEFALL3METHODS)
TA_K_OUT_CRT(TA_CDLSEPARATINGLINES)
TA_K_OUT_CRT(TA_CDLSHOOTINGSTAR)
TA_K_OUT_CRT(TA_CDLSHORTLINE)
TA_K_OUT_CRT(TA_CDLSPINNINGTOP)
TA_K_OUT_CRT(TA_CDLSTALLEDPATTERN)
TA_K_OUT_CRT(TA_CDLSTICKSANDWICH)
TA_K_OUT_CRT(TA_CDLTAKURI)
TA_K_OUT_CRT(TA_CDLTASUKIGAP)
TA_K_OUT_CRT(TA_CDLTHRUSTING)
TA_K_OUT_CRT(TA_CDLTRISTAR)
TA_K_OUT_CRT(TA_CDLUNIQUE3RIVER)
TA_K_OUT_CRT(TA_CDLUPSIDEGAP2CROWS)
TA_K_OUT_CRT(TA_CDLXSIDEGAP3METHODS)

TA_IN1_OUT_CRT(TA_CEIL)
TA_IN1_OUT_N_CRT(TA_CMO, 14)
TA_IN1_OUT_CRT(TA_COS)
TA_IN1_OUT_CRT(TA_COSH)
TA_IN1_OUT_N_CRT(TA_DEMA, 30)
TA_K_OUT_N_CRT(TA_DX, 14)
TA_IN1_OUT_N_CRT(TA_EMA, 30)
TA_IN1_OUT_CRT(TA_EXP)
TA_IN1_OUT_CRT(TA_FLOOR)
TA_IN1_OUT_CRT(TA_HT_DCPERIOD)
TA_IN1_OUT_CRT(TA_HT_DCPHASE)

Indicator HKU_API TA_HT_PHASOR();
inline Indicator TA_HT_PHASOR(const Indicator& ind) {
    return TA_HT_PHASOR()(ind);
}

Indicator HKU_API TA_HT_SINE();
inline Indicator TA_HT_SINE(const Indicator& ind) {
    return TA_HT_SINE()(ind);
}

TA_IN1_OUT_CRT(TA_HT_TRENDLINE)

Indicator HKU_API TA_HT_TRENDMODE();
inline Indicator TA_HT_TRENDMODE(const Indicator& ind) {
    return TA_HT_TRENDMODE()(ind);
}

TA_K_OUT_N_CRT(TA_IMI, 14)
TA_IN1_OUT_N_CRT(TA_KAMA, 30)
TA_IN1_OUT_N_CRT(TA_LINEARREG_ANGLE, 14)
TA_IN1_OUT_N_CRT(TA_LINEARREG_INTERCEPT, 14)
TA_IN1_OUT_N_CRT(TA_LINEARREG_SLOPE, 14)
TA_IN1_OUT_N_CRT(TA_LINEARREG, 14)
TA_IN1_OUT_CRT(TA_LN)
TA_IN1_OUT_CRT(TA_LOG10)

Indicator HKU_API TA_MA(int n = 30, int matype = 0);
inline Indicator TA_MA(const Indicator& ind, int n = 30, int matype = 0) {
    return TA_MA(n, matype)(ind);
}

Indicator HKU_API TA_MACD(int fast_n = 12, int slow_n = 26, int signal_n = 9);
inline Indicator TA_MACD(const Indicator& ind, int fast_n = 12, int slow_n = 26, int signal_n = 9) {
    return TA_MACD(fast_n, slow_n, signal_n)(ind);
}

Indicator HKU_API TA_MACDEXT(int fast_n = 12, int slow_n = 26, int signal_n = 9,
                             int fast_matype = 0, int slow_matype = 0, int signal_matype = 0);
inline Indicator TA_MACDEXT(const Indicator& ind, int fast_n = 12, int slow_n = 26,
                            int signal_n = 9, int fast_matype = 0, int slow_matype = 0,
                            int signal_matype = 0) {
    return TA_MACDEXT(fast_n, slow_n, signal_n, fast_matype, slow_matype, signal_matype)(ind);
}

Indicator HKU_API TA_MACDFIX(int n = 9);
inline Indicator TA_MACDFIX(const Indicator& ind, int n = 9) {
    return TA_MACDFIX(n)(ind);
}

Indicator HKU_API TA_MAMA(double fast_limit = 5.000000e-1, double slow_limit = 5.000000e-2);
inline Indicator TA_MAMA(const Indicator& ind, double fast_limit = 5.000000e-1,
                         double slow_limit = 5.000000e-2) {
    return TA_MAMA(fast_limit, slow_limit)(ind);
}

TA_IN1_OUT_N_CRT(TA_MAX, 30)

Indicator HKU_API TA_MAXINDEX(int n = 30);
inline Indicator TA_MAXINDEX(const Indicator& ind, int n = 30) {
    return TA_MAXINDEX(n)(ind);
}

TA_K_OUT_CRT(TA_MEDPRICE)
TA_K_OUT_N_CRT(TA_MFI, 14)
TA_IN1_OUT_N_CRT(TA_MIDPOINT, 14)

Indicator HKU_API TA_MIDPRICE(int n = 14);
Indicator HKU_API TA_MIDPRICE(const KData& k, int n = 14);

TA_IN1_OUT_N_CRT(TA_MIN, 30)

Indicator HKU_API TA_MININDEX(int n = 30);
inline Indicator TA_MININDEX(const Indicator& ind, int n = 30) {
    return TA_MININDEX(n)(ind);
}

TA_IN1_OUT_N_CRT(TA_MINMAX, 30)

Indicator HKU_API TA_MINMAXINDEX(int n = 30);
inline Indicator TA_MINMAXINDEX(const Indicator& ind, int n = 30) {
    return TA_MINMAXINDEX(n)(ind);
}

TA_K_OUT_N_CRT(TA_MINUS_DI, 14)
TA_K_OUT_N_CRT(TA_MINUS_DM, 14)
TA_IN1_OUT_N_CRT(TA_MOM, 10)
TA_K_OUT_N_CRT(TA_NATR, 14)
TA_K_OUT_N_CRT(TA_PLUS_DI, 14)
TA_K_OUT_N_CRT(TA_PLUS_DM, 14)
TA_IN1_OUT_N_CRT(TA_ROC, 10)
TA_IN1_OUT_N_CRT(TA_ROCP, 10)
TA_IN1_OUT_N_CRT(TA_ROCR, 10)
TA_IN1_OUT_N_CRT(TA_ROCR100, 10)
TA_IN1_OUT_N_CRT(TA_RSI, 14)
TA_IN1_OUT_CRT(TA_SIN)
TA_IN1_OUT_CRT(TA_SINH)
TA_IN1_OUT_N_CRT(TA_SMA, 30)
TA_IN1_OUT_CRT(TA_SQRT)
TA_IN1_OUT_CRT(TA_TAN)
TA_IN1_OUT_CRT(TA_TANH)
TA_IN1_OUT_N_CRT(TA_TEMA, 30)
TA_K_OUT_CRT(TA_TRANGE)
TA_IN1_OUT_N_CRT(TA_TRIMA, 30)
TA_IN1_OUT_N_CRT(TA_TRIX, 30)
TA_IN1_OUT_N_CRT(TA_TSF, 14)
TA_K_OUT_CRT(TA_TYPPRICE)
TA_K_OUT_CRT(TA_WCLPRICE)
TA_K_OUT_N_CRT(TA_WILLR, 14)
TA_IN1_OUT_N_CRT(TA_WMA, 30)

}  // namespace hku

#endif