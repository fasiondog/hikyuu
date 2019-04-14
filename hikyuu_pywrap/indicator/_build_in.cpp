/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/build_in.h>

using namespace hku;
using namespace boost::python;

Indicator (*PRICELIST2)(const PriceList&, int) = PRICELIST;
Indicator (*PRICELIST3)(const Indicator&, int) = PRICELIST;
Indicator (*PRICELIST4)(int) = PRICELIST;

Indicator (*KDATA1)(const KData&) = KDATA;
Indicator (*KDATA3)() = KDATA;

Indicator (*OPEN1)(const KData&) = OPEN;
Indicator (*OPEN3)() = OPEN;

Indicator (*HIGH1)(const KData&) = HIGH;
Indicator (*HIGH3)() = HIGH;

Indicator (*LOW1)(const KData&) = LOW;
Indicator (*LOW3)() = LOW;

Indicator (*CLOSE1)(const KData&) = CLOSE;
Indicator (*CLOSE3)() = CLOSE;

Indicator (*AMO1)(const KData&) = AMO;
Indicator (*AMO3)() = AMO;

Indicator (*VOL1)(const KData&) = VOL;
Indicator (*VOL3)() = VOL;

Indicator (*KDATA_PART1)(const KData& kdata, const string& part) = KDATA_PART;
Indicator (*KDATA_PART3)(const string& part) = KDATA_PART;

Indicator (*AMA_1)(int, int, int) = AMA;
Indicator (*AMA_2)(const Indicator&, int, int, int) = AMA;

Indicator (*ATR_1)(int) = ATR;
Indicator (*ATR_2)(const Indicator&, int) = ATR;

Indicator (*DIFF_1)() = DIFF;
Indicator (*DIFF_2)(const Indicator&) = DIFF;

Indicator (*MA_1)(int, const string&) = MA;
Indicator (*MA_2)(const Indicator&, int, const string&) = MA;
//BOOST_PYTHON_FUNCTION_OVERLOADS(MA_1_overload, MA, 1, 2);
//BOOST_PYTHON_FUNCTION_OVERLOADS(MA_2_overload, MA, 2, 3);


Indicator (*SMA_1)(int) = SMA;
Indicator (*SMA_2)(const Indicator&, int) = SMA;

Indicator (*EMA_1)(int) = EMA;
Indicator (*EMA_2)(const Indicator&, int) = EMA;

Indicator (*MACD_1)(int, int, int) = MACD;
Indicator (*MACD_2)(const Indicator&, int, int, int) = MACD;
//BOOST_PYTHON_FUNCTION_OVERLOADS(MACD_1_overload, MACD, 0, 3);
//BOOST_PYTHON_FUNCTION_OVERLOADS(MACD_2_overload, MACD, 1, 4);

Indicator (*REF_1)(int) = REF;
Indicator (*REF_2)(const Indicator&, int) = REF;

Indicator (*SAFTYLOSS_1)(int n1, int n2, double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_2)(const Indicator&, int n1, int n2, double p) = SAFTYLOSS;
//BOOST_PYTHON_FUNCTION_OVERLOADS(SAFTYLOSS_1_overload, SAFTYLOSS, 0, 3);
//BOOST_PYTHON_FUNCTION_OVERLOADS(SAFTYLOSS_2_overload, SAFTYLOSS, 1, 4);

Indicator (*STDEV_1)(int) = STDEV;
Indicator (*STDEV_2)(const Indicator&, int) = STDEV;

Indicator (*HHV_1)(int) = HHV;
Indicator (*HHV_2)(const Indicator&, int) = HHV;

Indicator (*LLV_1)(int) = LLV;
Indicator (*LLV_2)(const Indicator&, int) = LLV;

Indicator (*VIGOR_1)(const KData&, int) = VIGOR;
Indicator (*VIGOR_2)(int) = VIGOR;

Indicator (*CVAL_1)(double, size_t) = CVAL;
Indicator (*CVAL_2)(const Indicator&, double, int) = CVAL;

Indicator (*LIUTONGPAN_1)() = LIUTONGPAN;
Indicator (*LIUTONGPAN_2)(const KData&) = LIUTONGPAN;

Indicator (*HSL_1)() = HSL;
Indicator (*HSL_2)(const KData&) = HSL;

Indicator (*IF_1)(const Indicator&, const Indicator&, const Indicator&) = IF;
Indicator (*IF_2)(const Indicator&, price_t, const Indicator&) = IF;
Indicator (*IF_3)(const Indicator&, const Indicator&, price_t) = IF;
Indicator (*IF_4)(const Indicator&, price_t, price_t) = IF;

Indicator (*COUNT_1)(int) = COUNT;
Indicator (*COUNT_2)(const Indicator&, int) = COUNT;

Indicator (*SUM_1)(int) = SUM;
Indicator (*SUM_2)(const Indicator&, int) = SUM;

Indicator (*ABS_1)() = ABS;
Indicator (*ABS_2)(price_t) = ABS;
Indicator (*ABS_3)(const Indicator&) = ABS;

Indicator (*NOT_1)() = NOT;
Indicator (*NOT_2)(const Indicator&) = NOT;

Indicator (*SGN_1)() = SGN;
Indicator (*SGN_2)(price_t) = SGN;
Indicator (*SGN_3)(const Indicator&) = SGN;

Indicator (*EXP_1)() = EXP;
Indicator (*EXP_2)(price_t) = EXP;
Indicator (*EXP_3)(const Indicator&) = EXP;

Indicator (*MAX_1)(const Indicator&, const Indicator&) = MAX;
Indicator (*MAX_2)(const Indicator&, price_t) = MAX;
Indicator (*MAX_3)(price_t, const Indicator&) = MAX;

Indicator (*MIN_1)(const Indicator&, const Indicator&) = MIN;
Indicator (*MIN_2)(const Indicator&, price_t) = MIN;
Indicator (*MIN_3)(price_t, const Indicator&) = MIN;

Indicator (*BETWEEN_1)(const Indicator&, const Indicator&, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_2)(const Indicator&, const Indicator&, price_t) = BETWEEN;
Indicator (*BETWEEN_3)(const Indicator&, price_t, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_4)(const Indicator&, price_t, price_t) = BETWEEN;
Indicator (*BETWEEN_5)(price_t, const Indicator&, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_6)(price_t, const Indicator&, price_t) = BETWEEN;
Indicator (*BETWEEN_7)(price_t, price_t, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_8)(price_t, price_t, price_t) = BETWEEN;

Indicator (*LN_1)() = LN;
Indicator (*LN_2)(price_t) = LN;
Indicator (*LN_3)(const Indicator&) = LN;

Indicator (*LOG_1)() = LOG;
Indicator (*LOG_2)(price_t) = LOG;
Indicator (*LOG_3)(const Indicator&) = LOG;

Indicator (*HHVBARS_1)(int) = HHVBARS;
Indicator (*HHVBARS_2)(const Indicator&, int) = HHVBARS;

Indicator (*LLVBARS_1)(int) = LLVBARS;
Indicator (*LLVBARS_2)(const Indicator&, int) = LLVBARS;

Indicator (*POW_1)(int) = POW;
Indicator (*POW_2)(const Indicator&, int) = POW;
Indicator (*POW_3)(price_t, int) = POW;

Indicator (*SQRT_1)() = SQRT;
Indicator (*SQRT_2)(const Indicator&) = SQRT;
Indicator (*SQRT_3)(price_t) = SQRT;


void export_Indicator_build_in() {
    def("KDATA", KDATA1);
    def("KDATA", KDATA3);

    def("CLOSE", CLOSE1);
    def("CLOSE", CLOSE3);

    def("OPEN", OPEN1);
    def("OPEN", OPEN3);

    def("HIGH", HIGH1);
    def("HIGH", HIGH3);

    def("LOW", LOW1);
    def("LOW", LOW3);

    def("AMO", AMO1);
    def("AMO", AMO3);

    def("VOL", VOL1);
    def("VOL", VOL3);

    def("KDATA_PART", KDATA_PART1, (arg("data"), arg("kpart")));
    def("KDATA_PART", KDATA_PART3, (arg("kpart")));

    def("PRICELIST", PRICELIST2, (arg("data"), arg("discard")=0));
    def("PRICELIST", PRICELIST3, (arg("data"), arg("result_index")=0));
    def("PRICELIST", PRICELIST4, (arg("result_index")=0));

    def("SMA", SMA_1, (arg("n")=22));
    def("SMA", SMA_2, (arg("data"), arg("n")=22));
    def("EMA", EMA_1, (arg("n")=22));
    def("EMA", EMA_2, (arg("data"), arg("n")=22));

    def("MA", MA_1, (arg("n")=22, arg("type")="SMA"),"type='SMA|EMA|AMA'");
    def("MA", MA_2, (arg("data"), arg("n")=22, arg("type")="SMA"), "type='SMA|EMA|AMA'");

    def("AMA", AMA_1, (arg("n")=10, arg("fast_n")=2, arg("slow_n")=30));
    def("AMA", AMA_2, (arg("data"), arg("n")=10, arg("fast_n")=2, arg("slow_n")=30));

    def("ATR", ATR_1, (arg("n")=14));
    def("ATR", ATR_2, (arg("data"), arg("n")=14));

    def("MACD", MACD_1, (arg("n1")=12, arg("n2")=26, arg("n3")=9));
    def("MACD", MACD_2, (arg("data"), arg("n1")=12, arg("n2")=26, arg("n3")=9));

    def("VIGOR", VIGOR_1, (arg("kdata"), arg("n")=2));
    def("VIGOR", VIGOR_2, (arg("n")=2));

    def("SAFTYLOSS", SAFTYLOSS_1, (arg("n1")=10, arg("n2")=3, arg("p")=2.0));
    def("SAFTYLOSS", SAFTYLOSS_2, (arg("data"), arg("n1")=10, arg("n2")=3, arg("p")=2.0));
    def("DIFF", DIFF_1);
    def("DIFF", DIFF_2);

    def("REF", REF_1, (arg("n")));
    def("REF", REF_2, (arg("data"), arg("n")));

    def("STDEV", STDEV_1, (arg("n")=10));
    def("STDEV", STDEV_2, (arg("data"), arg("n")=10));

    def("POS", POS, (arg("block"), arg("query"), arg("sg")));

    def("HHV", HHV_1, (arg("n")=20));
    def("HHV", HHV_2, (arg("data"), arg("n")=20));

    def("LLV", LLV_1, (arg("n")=20));
    def("LLV", LLV_2, (arg("data"), arg("n")=20));

    def("CVAL", CVAL_1, (arg("value")=0.0, arg("discard")=0));
    def("CVAL", CVAL_2, (arg("data"), arg("value")=0.0, arg("discard")=0));

    def("LIUTONGPAN", LIUTONGPAN_1);
    def("LIUTONGPAN", LIUTONGPAN_2);
    def("HSL", HSL_1);
    def("HSL", HSL_2);

    def("WEAVE", WEAVE);
    def("IF", IF_1);
    def("IF", IF_2);
    def("IF", IF_3);
    def("IF", IF_4);

    def("COUNT", COUNT_1, (arg("n")=20));
    def("COUNT", COUNT_2, (arg("data"), arg("n")=20));

    def("SUM", SUM_1, (arg("n")=20));
    def("SUM", SUM_2, (arg("data"), arg("n")=20));

    def("ABS", ABS_1);
    def("ABS", ABS_2);
    def("ABS", ABS_3);

    def("NOT", NOT_1);
    def("NOT", NOT_2);

    def("SGN", SGN_1);
    def("SGN", SGN_2);
    def("SGN", SGN_3);

    def("EXP", EXP_1);
    def("EXP", EXP_2);
    def("EXP", EXP_3);

    def("MAX", MAX_1);
    def("MAX", MAX_2);
    def("MAX", MAX_3);

    def("MIN", MIN_1);
    def("MIN", MIN_2);
    def("MIN", MIN_3);

    def("BETWEEN", BETWEEN_1);
    def("BETWEEN", BETWEEN_2);
    def("BETWEEN", BETWEEN_3);
    def("BETWEEN", BETWEEN_4);
    def("BETWEEN", BETWEEN_5);
    def("BETWEEN", BETWEEN_6);
    def("BETWEEN", BETWEEN_7);
    def("BETWEEN", BETWEEN_8);

    def("LN", LN_1);
    def("LN", LN_2);
    def("LN", LN_3);

    def("LOG", LOG_1);
    def("LOG", LOG_2);
    def("LOG", LOG_3);

    def("HHVBARS", HHVBARS_1, (arg("n")=20));
    def("HHVBARS", HHVBARS_2, (arg("data"), arg("n")=20));

    def("LLVBARS", LLVBARS_1, (arg("n")=20));
    def("LLVBARS", LLVBARS_2, (arg("data"), arg("n")=20));

    def("POW", POW_1, (arg("n")));
    def("POW", POW_2, (arg("data"), arg("n")));
    def("POW", POW_3), (arg("data"), arg("n"));

    def("SQRT", SQRT_1);
    def("SQRT", SQRT_2);
    def("SQRT", SQRT_3);
}
