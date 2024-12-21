/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

#define TA_IN1_OUT_DEF(func)                                         \
    class Cls_##func : public IndicatorImp {                         \
        INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(Cls_##func)               \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
                                                                     \
    public:                                                          \
        Cls_##func();                                                \
        virtual ~Cls_##func();                                       \
        virtual void _checkParam(const string& name) const override; \
    };

#define TA_K_OUT_DEF(func)                            \
    class Cls_##func : public IndicatorImp {          \
        INDICATOR_IMP(Cls_##func)                     \
        INDICATOR_NEED_CONTEXT                        \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION \
    public:                                           \
        Cls_##func();                                 \
        explicit Cls_##func(const KData&);            \
        virtual ~Cls_##func() = default;              \
    };

#define TA_K_OUT_N_DEF(func)                                         \
    class Cls_##func : public IndicatorImp {                         \
        INDICATOR_IMP(Cls_##func)                                    \
        INDICATOR_NEED_CONTEXT                                       \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
    public:                                                          \
        Cls_##func();                                                \
        explicit Cls_##func(const KData&, int n);                    \
        virtual ~Cls_##func() = default;                             \
        virtual void _checkParam(const string& name) const override; \
    };

namespace hku {

TA_K_OUT_N_DEF(TA_ACCBANDS)
TA_IN1_OUT_DEF(TA_ACOS)
TA_K_OUT_DEF(TA_AD)
TA_K_OUT_N_DEF(TA_ADX)
TA_K_OUT_N_DEF(TA_ADXR)
TA_K_OUT_N_DEF(TA_AROON)
TA_K_OUT_N_DEF(TA_AROONOSC)
TA_IN1_OUT_DEF(TA_ASIN)
TA_IN1_OUT_DEF(TA_ATAN)
TA_K_OUT_N_DEF(TA_ATR)
TA_IN1_OUT_DEF(TA_AVGDEV)
TA_K_OUT_DEF(TA_AVGPRICE)
TA_K_OUT_DEF(TA_BOP)
TA_K_OUT_N_DEF(TA_CCI)
TA_IN1_OUT_DEF(TA_CEIL)
TA_IN1_OUT_DEF(TA_CMO)
TA_IN1_OUT_DEF(TA_COS)
TA_IN1_OUT_DEF(TA_COSH)
TA_IN1_OUT_DEF(TA_DEMA)
TA_IN1_OUT_DEF(TA_EMA)
TA_IN1_OUT_DEF(TA_EXP)
TA_IN1_OUT_DEF(TA_FLOOR)
TA_IN1_OUT_DEF(TA_HT_DCPERIOD)
TA_IN1_OUT_DEF(TA_HT_DCPHASE)
TA_IN1_OUT_DEF(TA_HT_TRENDLINE)
TA_IN1_OUT_DEF(TA_KAMA)
TA_IN1_OUT_DEF(TA_LINEARREG_ANGLE)
TA_IN1_OUT_DEF(TA_LINEARREG_INTERCEPT)
TA_IN1_OUT_DEF(TA_LINEARREG_SLOPE)
TA_IN1_OUT_DEF(TA_LINEARREG)
TA_IN1_OUT_DEF(TA_LN)
TA_IN1_OUT_DEF(TA_LOG10)
TA_IN1_OUT_DEF(TA_MAX)
TA_K_OUT_DEF(TA_MEDPRICE)
TA_IN1_OUT_DEF(TA_MIDPOINT)
TA_IN1_OUT_DEF(TA_MIN)
TA_IN1_OUT_DEF(TA_MINMAX)
TA_IN1_OUT_DEF(TA_MOM)
TA_IN1_OUT_DEF(TA_ROC)
TA_IN1_OUT_DEF(TA_ROCP)
TA_IN1_OUT_DEF(TA_ROCR)
TA_IN1_OUT_DEF(TA_ROCR100)
TA_IN1_OUT_DEF(TA_RSI)
TA_IN1_OUT_DEF(TA_SIN)
TA_IN1_OUT_DEF(TA_SINH)
TA_IN1_OUT_DEF(TA_SMA)
TA_IN1_OUT_DEF(TA_SQRT)
TA_IN1_OUT_DEF(TA_TAN)
TA_IN1_OUT_DEF(TA_TANH)
TA_IN1_OUT_DEF(TA_TEMA)
TA_IN1_OUT_DEF(TA_TRIMA)
TA_IN1_OUT_DEF(TA_TRIX)
TA_IN1_OUT_DEF(TA_TSF)
TA_IN1_OUT_DEF(TA_WMA)

}  // namespace hku