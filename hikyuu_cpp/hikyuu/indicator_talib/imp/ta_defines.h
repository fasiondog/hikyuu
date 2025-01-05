/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

#define TA_IN1_OUT_DEF(func)                          \
    class Cls_##func : public IndicatorImp {          \
        INDICATOR_IMP(Cls_##func)                     \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION \
                                                      \
    public:                                           \
        Cls_##func();                                 \
        virtual ~Cls_##func();                        \
    };

#define TA_IN1_OUT_DYN_DEF(func)                                     \
    class Cls_##func : public IndicatorImp {                         \
        INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(Cls_##func)               \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
                                                                     \
    public:                                                          \
        Cls_##func();                                                \
        virtual ~Cls_##func();                                       \
        virtual void _checkParam(const string& name) const override; \
    };

#if HKU_SUPPORT_SERIALIZATION
#define TA_IN2_OUT_DEF(func)                                       \
    class Cls_##func : public IndicatorImp {                       \
    public:                                                        \
        Cls_##func();                                              \
        Cls_##func(const Indicator& ref_ind, bool fill_null);      \
        virtual ~Cls_##func();                                     \
        virtual void _calculate(const Indicator& data) override;   \
        virtual IndicatorImpPtr _clone() override;                 \
                                                                   \
    private:                                                       \
        Indicator m_ref_ind;                                       \
                                                                   \
    private:                                                       \
        friend class boost::serialization::access;                 \
        template <class Archive>                                   \
        void serialize(Archive& ar, const unsigned int version) {  \
            ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp); \
            ar& BOOST_SERIALIZATION_NVP(m_ref_ind);                \
        }                                                          \
    };

#define TA_IN2_OUT_N_DEF(func)                                       \
    class Cls_##func : public IndicatorImp {                         \
    public:                                                          \
        Cls_##func();                                                \
        explicit Cls_##func(int n, bool fill_null);                  \
        Cls_##func(const Indicator& ref_ind, int n, bool fill_null); \
        virtual ~Cls_##func();                                       \
        virtual void _checkParam(const string& name) const override; \
        virtual void _calculate(const Indicator& data) override;     \
        virtual IndicatorImpPtr _clone() override;                   \
                                                                     \
    private:                                                         \
        Indicator m_ref_ind;                                         \
                                                                     \
    private:                                                         \
        friend class boost::serialization::access;                   \
        template <class Archive>                                     \
        void serialize(Archive& ar, const unsigned int version) {    \
            ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);   \
            ar& BOOST_SERIALIZATION_NVP(m_ref_ind);                  \
        }                                                            \
    };
#else
#define TA_IN2_OUT_DEF(func)                                     \
    class Cls_##func : public IndicatorImp {                     \
    public:                                                      \
        Cls_##func();                                            \
        Cls_##func(const Indicator& ref_ind);                    \
        virtual ~Cls_##func();                                   \
        virtual void _calculate(const Indicator& data) override; \
        virtual IndicatorImpPtr _clone() override;               \
                                                                 \
    private:                                                     \
        Indicator m_ref_ind;                                     \
    };

#define TA_IN2_OUT_N_DEF(func)                                       \
    class Cls_##func : public IndicatorImp {                         \
    public:                                                          \
        Cls_##func();                                                \
        explicit Cls_##func(int n);                                  \
        Cls_##func(const Indicator& ref_ind, int n);                 \
        virtual ~Cls_##func();                                       \
        virtual void _checkParam(const string& name) const override; \
        virtual void _calculate(const Indicator& data) override;     \
        virtual IndicatorImpPtr _clone() override;                   \
                                                                     \
    private:                                                         \
        Indicator m_ref_ind;                                         \
    };
#endif

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

#define TA_K_OUT_P_D_DEF(func)                                       \
    class Cls_##func : public IndicatorImp {                         \
        INDICATOR_IMP(Cls_##func)                                    \
        INDICATOR_NEED_CONTEXT                                       \
        INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
    public:                                                          \
        Cls_##func();                                                \
        explicit Cls_##func(const KData&, double p);                 \
        virtual ~Cls_##func() = default;                             \
        virtual void _checkParam(const string& name) const override; \
    };

namespace hku {

TA_K_OUT_N_DEF(TA_ACCBANDS)
TA_IN1_OUT_DEF(TA_ACOS)
TA_K_OUT_DEF(TA_AD)
TA_IN2_OUT_DEF(TA_ADD)
TA_K_OUT_N_DEF(TA_ADX)
TA_K_OUT_N_DEF(TA_ADXR)
TA_K_OUT_N_DEF(TA_AROON)
TA_K_OUT_N_DEF(TA_AROONOSC)
TA_IN1_OUT_DEF(TA_ASIN)
TA_IN1_OUT_DEF(TA_ATAN)
TA_K_OUT_N_DEF(TA_ATR)
TA_IN1_OUT_DYN_DEF(TA_AVGDEV)
TA_K_OUT_DEF(TA_AVGPRICE)
TA_IN2_OUT_N_DEF(TA_BETA)
TA_K_OUT_DEF(TA_BOP)
TA_K_OUT_N_DEF(TA_CCI)
TA_K_OUT_DEF(TA_CDL2CROWS)
TA_K_OUT_DEF(TA_CDL3BLACKCROWS)
TA_K_OUT_DEF(TA_CDL3INSIDE)
TA_K_OUT_DEF(TA_CDL3LINESTRIKE)
TA_K_OUT_DEF(TA_CDL3OUTSIDE)
TA_K_OUT_DEF(TA_CDL3STARSINSOUTH)
TA_K_OUT_DEF(TA_CDL3WHITESOLDIERS)
TA_K_OUT_P_D_DEF(TA_CDLABANDONEDBABY)
TA_K_OUT_DEF(TA_CDLADVANCEBLOCK)
TA_K_OUT_DEF(TA_CDLBELTHOLD)
TA_K_OUT_DEF(TA_CDLBREAKAWAY)
TA_K_OUT_DEF(TA_CDLCLOSINGMARUBOZU)
TA_K_OUT_DEF(TA_CDLCONCEALBABYSWALL)
TA_K_OUT_DEF(TA_CDLCOUNTERATTACK)
TA_K_OUT_P_D_DEF(TA_CDLDARKCLOUDCOVER)
TA_K_OUT_DEF(TA_CDLDOJI)
TA_K_OUT_DEF(TA_CDLDOJISTAR)
TA_K_OUT_DEF(TA_CDLDRAGONFLYDOJI)
TA_K_OUT_DEF(TA_CDLENGULFING)
TA_K_OUT_P_D_DEF(TA_CDLEVENINGDOJISTAR)
TA_K_OUT_P_D_DEF(TA_CDLEVENINGSTAR)
TA_K_OUT_DEF(TA_CDLGAPSIDESIDEWHITE)
TA_K_OUT_DEF(TA_CDLGRAVESTONEDOJI)
TA_K_OUT_DEF(TA_CDLHAMMER)
TA_K_OUT_DEF(TA_CDLHANGINGMAN)
TA_K_OUT_DEF(TA_CDLHARAMI)
TA_K_OUT_DEF(TA_CDLHARAMICROSS)
TA_K_OUT_DEF(TA_CDLHIGHWAVE)
TA_K_OUT_DEF(TA_CDLHIKKAKE)
TA_K_OUT_DEF(TA_CDLHIKKAKEMOD)
TA_K_OUT_DEF(TA_CDLHOMINGPIGEON)
TA_K_OUT_DEF(TA_CDLIDENTICAL3CROWS)
TA_K_OUT_DEF(TA_CDLINNECK)
TA_K_OUT_DEF(TA_CDLINVERTEDHAMMER)
TA_K_OUT_DEF(TA_CDLKICKING)
TA_K_OUT_DEF(TA_CDLKICKINGBYLENGTH)
TA_K_OUT_DEF(TA_CDLLADDERBOTTOM)
TA_K_OUT_DEF(TA_CDLLONGLEGGEDDOJI)
TA_K_OUT_DEF(TA_CDLLONGLINE)
TA_K_OUT_DEF(TA_CDLMARUBOZU)
TA_K_OUT_DEF(TA_CDLMATCHINGLOW)
TA_K_OUT_P_D_DEF(TA_CDLMATHOLD)
TA_K_OUT_P_D_DEF(TA_CDLMORNINGDOJISTAR)
TA_K_OUT_P_D_DEF(TA_CDLMORNINGSTAR)
TA_K_OUT_DEF(TA_CDLONNECK)
TA_K_OUT_DEF(TA_CDLPIERCING)
TA_K_OUT_DEF(TA_CDLRICKSHAWMAN)
TA_K_OUT_DEF(TA_CDLRISEFALL3METHODS)
TA_K_OUT_DEF(TA_CDLSEPARATINGLINES)
TA_K_OUT_DEF(TA_CDLSHOOTINGSTAR)
TA_K_OUT_DEF(TA_CDLSHORTLINE)
TA_K_OUT_DEF(TA_CDLSPINNINGTOP)
TA_K_OUT_DEF(TA_CDLSTALLEDPATTERN)
TA_K_OUT_DEF(TA_CDLSTICKSANDWICH)
TA_K_OUT_DEF(TA_CDLTAKURI)
TA_K_OUT_DEF(TA_CDLTASUKIGAP)
TA_K_OUT_DEF(TA_CDLTHRUSTING)
TA_K_OUT_DEF(TA_CDLTRISTAR)
TA_K_OUT_DEF(TA_CDLUNIQUE3RIVER)
TA_K_OUT_DEF(TA_CDLUPSIDEGAP2CROWS)
TA_K_OUT_DEF(TA_CDLXSIDEGAP3METHODS)
TA_IN1_OUT_DEF(TA_CEIL)
TA_IN1_OUT_DYN_DEF(TA_CMO)
TA_IN2_OUT_N_DEF(TA_CORREL)
TA_IN1_OUT_DEF(TA_COS)
TA_IN1_OUT_DEF(TA_COSH)
TA_IN1_OUT_DYN_DEF(TA_DEMA)
TA_IN2_OUT_DEF(TA_DIV)
TA_K_OUT_N_DEF(TA_DX)
TA_IN1_OUT_DYN_DEF(TA_EMA)
TA_IN1_OUT_DEF(TA_EXP)
TA_IN1_OUT_DEF(TA_FLOOR)
TA_IN1_OUT_DEF(TA_HT_DCPERIOD)
TA_IN1_OUT_DEF(TA_HT_DCPHASE)
TA_IN1_OUT_DEF(TA_HT_PHASOR)
TA_IN1_OUT_DEF(TA_HT_SINE)
TA_IN1_OUT_DEF(TA_HT_TRENDLINE)
TA_IN1_OUT_DEF(TA_HT_TRENDMODE)
TA_K_OUT_N_DEF(TA_IMI)
TA_IN1_OUT_DYN_DEF(TA_KAMA)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_ANGLE)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_INTERCEPT)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_SLOPE)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG)
TA_IN1_OUT_DEF(TA_LN)
TA_IN1_OUT_DEF(TA_LOG10)
TA_IN1_OUT_DYN_DEF(TA_MACDFIX)
TA_IN1_OUT_DYN_DEF(TA_MAX)
TA_IN1_OUT_DYN_DEF(TA_MAXINDEX)
TA_K_OUT_DEF(TA_MEDPRICE)
TA_K_OUT_N_DEF(TA_MFI)
TA_IN1_OUT_DYN_DEF(TA_MIDPOINT)
TA_K_OUT_N_DEF(TA_MIDPRICE)
TA_IN1_OUT_DYN_DEF(TA_MIN)
TA_IN1_OUT_DYN_DEF(TA_MININDEX)
TA_IN1_OUT_DYN_DEF(TA_MINMAX)
TA_IN1_OUT_DYN_DEF(TA_MINMAXINDEX)
TA_K_OUT_N_DEF(TA_MINUS_DI)
TA_K_OUT_N_DEF(TA_MINUS_DM)
TA_IN1_OUT_DYN_DEF(TA_MOM)
TA_IN2_OUT_DEF(TA_MULT)
TA_K_OUT_N_DEF(TA_NATR)
TA_K_OUT_DEF(TA_OBV)
TA_K_OUT_N_DEF(TA_PLUS_DI)
TA_K_OUT_N_DEF(TA_PLUS_DM)
TA_IN1_OUT_DYN_DEF(TA_ROC)
TA_IN1_OUT_DYN_DEF(TA_ROCP)
TA_IN1_OUT_DYN_DEF(TA_ROCR)
TA_IN1_OUT_DYN_DEF(TA_ROCR100)
TA_IN1_OUT_DYN_DEF(TA_RSI)
TA_IN1_OUT_DEF(TA_SIN)
TA_IN1_OUT_DEF(TA_SINH)
TA_IN1_OUT_DYN_DEF(TA_SMA)
TA_IN1_OUT_DEF(TA_SQRT)
TA_IN2_OUT_DEF(TA_SUB)
TA_IN1_OUT_DYN_DEF(TA_SUM)
TA_IN1_OUT_DEF(TA_TAN)
TA_IN1_OUT_DEF(TA_TANH)
TA_IN1_OUT_DYN_DEF(TA_TEMA)
TA_K_OUT_DEF(TA_TRANGE)
TA_IN1_OUT_DYN_DEF(TA_TRIMA)
TA_IN1_OUT_DYN_DEF(TA_TRIX)
TA_IN1_OUT_DYN_DEF(TA_TSF)
TA_K_OUT_DEF(TA_TYPPRICE)
TA_K_OUT_DEF(TA_WCLPRICE)
TA_K_OUT_N_DEF(TA_WILLR)
TA_IN1_OUT_DYN_DEF(TA_WMA)

}  // namespace hku