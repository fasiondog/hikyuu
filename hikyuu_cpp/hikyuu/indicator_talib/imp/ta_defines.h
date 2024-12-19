/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

#define TA_IN1_OUT1_N_DEF(func)                                      \
    class Cls_##func : public IndicatorImp {                         \
        INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(Cls_##func)               \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
                                                                     \
    public:                                                          \
        Cls_##func();                                                \
        virtual ~Cls_##func();                                       \
        virtual void _checkParam(const string& name) const override; \
    };

namespace hku {

TA_IN1_OUT1_N_DEF(TA_AVGDEV)
TA_IN1_OUT1_N_DEF(TA_CMO)
TA_IN1_OUT1_N_DEF(TA_DEMA)
TA_IN1_OUT1_N_DEF(TA_EMA)
TA_IN1_OUT1_N_DEF(TA_KAMA)
TA_IN1_OUT1_N_DEF(TA_LINEARREG_ANGLE)
TA_IN1_OUT1_N_DEF(TA_MAX)
TA_IN1_OUT1_N_DEF(TA_MIDPOINT)
TA_IN1_OUT1_N_DEF(TA_MIN)
TA_IN1_OUT1_N_DEF(TA_MOM)
TA_IN1_OUT1_N_DEF(TA_ROC)
TA_IN1_OUT1_N_DEF(TA_ROCP)
TA_IN1_OUT1_N_DEF(TA_ROCR)
TA_IN1_OUT1_N_DEF(TA_ROCR100)
TA_IN1_OUT1_N_DEF(TA_RSI)
TA_IN1_OUT1_N_DEF(TA_SMA)
TA_IN1_OUT1_N_DEF(TA_TEMA)
TA_IN1_OUT1_N_DEF(TA_TRIMA)
TA_IN1_OUT1_N_DEF(TA_TRIX)
TA_IN1_OUT1_N_DEF(TA_TSF)
TA_IN1_OUT1_N_DEF(TA_WMA)

}  // namespace hku