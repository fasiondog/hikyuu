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

#define TA_IN1_OUT1_CRT(func)                               \
    Indicator HKU_API func();                               \
    inline Indicator func(const Indicator& ind) {           \
        return func()(ind);                                 \
    }                                                       \
    inline Indicator HKU_API func(Indicator::value_t val) { \
        return func(CVAL(val));                             \
    }

#define TA_IN1_OUT1_N_CRT(func, period)                                        \
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

namespace hku {

Indicator HKU_API TA_ACCBANDS();
Indicator HKU_API TA_ACCBANDS(const KData& k, int n = 20);

TA_IN1_OUT1_CRT(TA_ACOS)

Indicator HKU_API TA_AD();
Indicator HKU_API TA_AD(const KData& k);

Indicator HKU_API TA_ADOSC();
Indicator HKU_API TA_ADOSC(const KData& k, int fast_n = 3, int slow_n = 10);

TA_IN1_OUT1_CRT(TA_ASIN)
TA_IN1_OUT1_CRT(TA_ATAN)
TA_IN1_OUT1_N_CRT(TA_AVGDEV, 14)
TA_IN1_OUT1_CRT(TA_CEIL)
TA_IN1_OUT1_N_CRT(TA_CMO, 14)
TA_IN1_OUT1_CRT(TA_COS)
TA_IN1_OUT1_CRT(TA_COSH)
TA_IN1_OUT1_N_CRT(TA_DEMA, 30)
TA_IN1_OUT1_N_CRT(TA_EMA, 30)
TA_IN1_OUT1_CRT(TA_EXP)
TA_IN1_OUT1_CRT(TA_FLOOR)
TA_IN1_OUT1_CRT(TA_HT_DCPERIOD)
TA_IN1_OUT1_CRT(TA_HT_DCPHASE)
TA_IN1_OUT1_CRT(TA_HT_TRENDLINE)
TA_IN1_OUT1_N_CRT(TA_KAMA, 30)
TA_IN1_OUT1_N_CRT(TA_LINEARREG_ANGLE, 14)
TA_IN1_OUT1_N_CRT(TA_LINEARREG_INTERCEPT, 14)
TA_IN1_OUT1_N_CRT(TA_LINEARREG_SLOPE, 14)
TA_IN1_OUT1_N_CRT(TA_LINEARREG, 14)
TA_IN1_OUT1_CRT(TA_LN)
TA_IN1_OUT1_CRT(TA_LOG10)
TA_IN1_OUT1_N_CRT(TA_MAX, 30)
TA_IN1_OUT1_N_CRT(TA_MIDPOINT, 14)
TA_IN1_OUT1_N_CRT(TA_MIN, 30)
TA_IN1_OUT1_N_CRT(TA_MOM, 10)
TA_IN1_OUT1_N_CRT(TA_ROC, 10)
TA_IN1_OUT1_N_CRT(TA_ROCP, 10)
TA_IN1_OUT1_N_CRT(TA_ROCR, 10)
TA_IN1_OUT1_N_CRT(TA_ROCR100, 10)
TA_IN1_OUT1_N_CRT(TA_RSI, 14)
TA_IN1_OUT1_CRT(TA_SIN)
TA_IN1_OUT1_CRT(TA_SINH)
TA_IN1_OUT1_N_CRT(TA_SMA, 30)
TA_IN1_OUT1_CRT(TA_SQRT)
TA_IN1_OUT1_CRT(TA_TAN)
TA_IN1_OUT1_CRT(TA_TANH)
TA_IN1_OUT1_N_CRT(TA_TEMA, 30)
TA_IN1_OUT1_N_CRT(TA_TRIMA, 30)
TA_IN1_OUT1_N_CRT(TA_TRIX, 30)
TA_IN1_OUT1_N_CRT(TA_TSF, 14)
TA_IN1_OUT1_N_CRT(TA_WMA, 30)

}  // namespace hku

#endif