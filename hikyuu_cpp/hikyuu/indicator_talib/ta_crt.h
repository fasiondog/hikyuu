/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/config.h"
#include "hikyuu/indicator/Indicator.h"

#if HKU_ENABLE_TA_LIB

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

TA_IN1_OUT1_N_CRT(TA_CMO, 14)
TA_IN1_OUT1_N_CRT(TA_DEMA, 30)
TA_IN1_OUT1_N_CRT(TA_EMA, 30)
TA_IN1_OUT1_N_CRT(TA_AVGDEV, 14)
TA_IN1_OUT1_N_CRT(TA_KAMA, 30)
TA_IN1_OUT1_N_CRT(TA_LINEARREG_ANGLE, 14)
TA_IN1_OUT1_N_CRT(TA_MAX, 30)
TA_IN1_OUT1_N_CRT(TA_MIDPOINT, 14)
TA_IN1_OUT1_N_CRT(TA_MIN, 30)
TA_IN1_OUT1_N_CRT(TA_MOM, 10)
TA_IN1_OUT1_N_CRT(TA_ROC, 10)
TA_IN1_OUT1_N_CRT(TA_ROCP, 10)
TA_IN1_OUT1_N_CRT(TA_ROCR, 10)
TA_IN1_OUT1_N_CRT(TA_ROCR100, 10)
TA_IN1_OUT1_N_CRT(TA_RSI, 14)
TA_IN1_OUT1_N_CRT(TA_SMA, 30)
TA_IN1_OUT1_N_CRT(TA_TEMA, 30)
TA_IN1_OUT1_N_CRT(TA_TRIMA, 30)
TA_IN1_OUT1_N_CRT(TA_TRIX, 30)
TA_IN1_OUT1_N_CRT(TA_TSF, 14)
TA_IN1_OUT1_N_CRT(TA_WMA, 30)

}  // namespace hku

#endif