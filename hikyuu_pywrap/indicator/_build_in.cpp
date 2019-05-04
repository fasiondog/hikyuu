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

Indicator (*STDP_1)(int) = STDP;
Indicator (*STDP_2)(const Indicator&, int) = STDP;

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

Indicator (*ROUND_1)(int) = ROUND;
Indicator (*ROUND_2)(const Indicator&, int) = ROUND;
Indicator (*ROUND_3)(price_t, int) = ROUND;

Indicator (*ROUNDUP_1)(int) = ROUNDUP;
Indicator (*ROUNDUP_2)(const Indicator&, int) = ROUNDUP;
Indicator (*ROUNDUP_3)(price_t, int) = ROUNDUP;

Indicator (*ROUNDDOWN_1)(int) = ROUNDDOWN;
Indicator (*ROUNDDOWN_2)(const Indicator&, int) = ROUNDDOWN;
Indicator (*ROUNDDOWN_3)(price_t, int) = ROUNDDOWN;

Indicator (*FLOOR_1)() = FLOOR;
Indicator (*FLOOR_2)(const Indicator&) = FLOOR;
Indicator (*FLOOR_3)(price_t) = FLOOR;

Indicator (*CEILING_1)() = CEILING;
Indicator (*CEILING_2)(const Indicator&) = CEILING;
Indicator (*CEILING_3)(price_t) = CEILING;

Indicator (*INTPART_1)() = INTPART;
Indicator (*INTPART_2)(const Indicator&) = INTPART;
Indicator (*INTPART_3)(price_t) = INTPART;

Indicator (*EXIST_1)(int) = EXIST;
Indicator (*EXIST_2)(const Indicator&, int) = EXIST;

Indicator (*EVERY_1)(int) = EVERY;
Indicator (*EVERY_2)(const Indicator&, int) = EVERY;

Indicator (*LAST_1)(int, int) = LAST;
Indicator (*LAST_2)(const Indicator&, int, int) = LAST;

Indicator (*SIN_1)() = SIN;
Indicator (*SIN_2)(const Indicator&) = SIN;
Indicator (*SIN_3)(price_t) = SIN;

Indicator (*ASIN_1)() = ASIN;
Indicator (*ASIN_2)(const Indicator&) = ASIN;
Indicator (*ASIN_3)(price_t) = ASIN;

Indicator (*COS_1)() = COS;
Indicator (*COS_2)(const Indicator&) = COS;
Indicator (*COS_3)(price_t) = COS;

Indicator (*ACOS_1)() = ACOS;
Indicator (*ACOS_2)(const Indicator&) = ACOS;
Indicator (*ACOS_3)(price_t) = ACOS;

Indicator (*TAN_1)() = TAN;
Indicator (*TAN_2)(const Indicator&) = TAN;
Indicator (*TAN_3)(price_t) = TAN;

Indicator (*ATAN_1)() = ATAN;
Indicator (*ATAN_2)(const Indicator&) = ATAN;
Indicator (*ATAN_3)(price_t) = ATAN;

Indicator (*REVERSE_1)() = REVERSE;
Indicator (*REVERSE_2)(const Indicator&) = REVERSE;
Indicator (*REVERSE_3)(price_t) = REVERSE;

Indicator (*MOD_1)(const Indicator&, const Indicator&) = MOD;
Indicator (*MOD_2)(const Indicator&, price_t) = MOD;
Indicator (*MOD_3)(price_t, const Indicator&) = MOD;
Indicator (*MOD_4)(price_t, price_t) = MOD;

Indicator (*VAR_1)(int) = VAR;
Indicator (*VAR_2)(const Indicator&, int) = VAR;

Indicator (*VARP_1)(int) = VARP;
Indicator (*VARP_2)(const Indicator&, int) = VARP;

Indicator (*CROSS_1)(const Indicator&, const Indicator&) = CROSS;
Indicator (*CROSS_2)(const Indicator&, price_t) = CROSS;
Indicator (*CROSS_3)(price_t, const Indicator&) = CROSS;
Indicator (*CROSS_4)(price_t, price_t) = CROSS;

Indicator (*LONGCROSS_1)(const Indicator&, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_2)(const Indicator&, price_t, int) = LONGCROSS;
Indicator (*LONGCROSS_3)(price_t, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_4)(price_t, price_t, int) = LONGCROSS;

Indicator (*FILTER_1)(int) = FILTER;
Indicator (*FILTER_2)(const Indicator&, int) = FILTER;


void export_Indicator_build_in() {
    def("IKDATA", KDATA1);
    def("IKDATA", KDATA3);

    def("ICLOSE", CLOSE1);
    def("ICLOSE", CLOSE3);

    def("IOPEN", OPEN1);
    def("IOPEN", OPEN3);

    def("IHIGH", HIGH1);
    def("IHIGH", HIGH3);

    def("ILOW", LOW1);
    def("ILOW", LOW3);

    def("IAMO", AMO1);
    def("IAMO", AMO3);

    def("IVOL", VOL1);
    def("IVOL", VOL3);

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

    def("STDP", STDP_1, (arg("n")=10));
    def("STDP", STDP_2, (arg("data"), arg("n")=10));

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

    def("ROUND", ROUND_1, (arg("ndigits") = 2));
    def("ROUND", ROUND_2, (arg("data"), arg("ndigits")=2));
    def("ROUND", ROUND_3, (arg("data"), arg("ndigits")=2));

    def("ROUNDUP", ROUNDUP_1, (arg("ndigits") = 2));
    def("ROUNDUP", ROUNDUP_2, (arg("data"), arg("ndigits")=2));
    def("ROUNDUP", ROUNDUP_3, (arg("data"), arg("ndigits")=2));

    def("ROUNDDOWN", ROUNDDOWN_1, (arg("ndigits") = 2));
    def("ROUNDDOWN", ROUNDDOWN_2, (arg("data"), arg("ndigits")=2));
    def("ROUNDDOWN", ROUNDDOWN_3, (arg("data"), arg("ndigits")=2));

    def("FLOOR", FLOOR_1);
    def("FLOOR", FLOOR_2);
    def("FLOOR", FLOOR_3);

    def("CEILING", CEILING_1);
    def("CEILING", CEILING_2);
    def("CEILING", CEILING_3);

    def("INTPART", INTPART_1);
    def("INTPART", INTPART_2);
    def("INTPART", INTPART_3);

    def("EXIST", EXIST_1, (arg("n")=20));
    def("EXIST", EXIST_2, (arg("data"), arg("n")=20));

    def("EVERY", EVERY_1, (arg("n")=20));
    def("EVERY", EVERY_2, (arg("data"), arg("n")=20));

    def("LAST", LAST_1, (arg("m")=10, arg("n")=5));
    def("LAST", LAST_2, (arg("data"), arg("m")=10, arg("n")=5));

    def("SIN", SIN_1);
    def("SIN", SIN_2);
    def("SIN", SIN_3);

    def("ASIN", ASIN_1);
    def("ASIN", ASIN_2);
    def("ASIN", ASIN_3); 

    def("COS", COS_1);
    def("COS", COS_2);
    def("COS", COS_3); 

    def("ACOS", ACOS_1);
    def("ACOS", ACOS_2);
    def("ACOS", ACOS_3); 

    def("TAN", TAN_1);
    def("TAN", TAN_2);
    def("TAN", TAN_3); 

    def("ATAN", ATAN_1);
    def("ATAN", ATAN_2);
    def("ATAN", ATAN_3); 

    def("REVERSE", REVERSE_1);
    def("REVERSE", REVERSE_2);
    def("REVERSE", REVERSE_3);

    def("MOD", MOD_1);
    def("MOD", MOD_2);
    def("MOD", MOD_3);
    def("MOD", MOD_4);

    def("VAR", VAR_1, (arg("n")=10));
    def("VAR", VAR_2, (arg("data"), arg("n")=10));

    def("VARP", VARP_1, (arg("n")=10));
    def("VARP", VARP_2, (arg("data"), arg("n")=10));

    def("UPNDAY", UPNDAY, (arg("data"), arg("n")=3));
    def("DOWNNDAY", DOWNNDAY, (arg("data"), arg("n")=3));
    def("NDAY", NDAY, (arg("x"), arg("y"), arg("n")=3));

    def("CROSS", CROSS_1);
    def("CROSS", CROSS_2);
    def("CROSS", CROSS_3);
    def("CROSS", CROSS_4);

    def("LONGCROSS", LONGCROSS_1, (arg("a"), arg("b"), arg("n")=3));
    def("LONGCROSS", LONGCROSS_2, (arg("a"), arg("b"), arg("n")=3));
    def("LONGCROSS", LONGCROSS_3, (arg("a"), arg("b"), arg("n")=3));
    def("LONGCROSS", LONGCROSS_4, (arg("a"), arg("b"), arg("n")=3));

    def("FILTER", FILTER_1, (arg("n")=5));
    def("FILTER", FILTER_2, (arg("data"), arg("n")=5));
}