/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <hikyuu/indicator/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

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

Indicator (*ISLIMITUP1)() = ISLIMITUP;
Indicator (*ISLIMITUP2)(const KData&) = ISLIMITUP;

Indicator (*ISLIMITDOWN1)() = ISLIMITDOWN;
Indicator (*ISLIMITDOWN2)(const KData&) = ISLIMITDOWN;

Indicator (*KDATA_PART1)(const KData& kdata, const string& part) = KDATA_PART;
Indicator (*KDATA_PART3)(const string& part) = KDATA_PART;

Indicator (*RECOVER_FORWARD_1)() = RECOVER_FORWARD;
Indicator (*RECOVER_FORWARD_2)(const Indicator&) = RECOVER_FORWARD;
Indicator (*RECOVER_FORWARD_3)(const KData&) = RECOVER_FORWARD;

Indicator (*RECOVER_BACKWARD_1)() = RECOVER_BACKWARD;
Indicator (*RECOVER_BACKWARD_2)(const Indicator&) = RECOVER_BACKWARD;
Indicator (*RECOVER_BACKWARD_3)(const KData&) = RECOVER_BACKWARD;

Indicator (*STKTYPE1)() = STKTYPE;
Indicator (*STKTYPE2)(const KData&) = STKTYPE;

Indicator (*RECOVER_EQUAL_FORWARD_1)() = RECOVER_EQUAL_FORWARD;
Indicator (*RECOVER_EQUAL_FORWARD_2)(const Indicator&) = RECOVER_EQUAL_FORWARD;
Indicator (*RECOVER_EQUAL_FORWARD_3)(const KData&) = RECOVER_EQUAL_FORWARD;

Indicator (*RECOVER_EQUAL_BACKWARD_1)() = RECOVER_EQUAL_BACKWARD;
Indicator (*RECOVER_EQUAL_BACKWARD_2)(const Indicator&) = RECOVER_EQUAL_BACKWARD;
Indicator (*RECOVER_EQUAL_BACKWARD_3)(const KData&) = RECOVER_EQUAL_BACKWARD;

Indicator (*DATE1)() = hku::DATE;
Indicator (*DATE2)(const KData&) = hku::DATE;

Indicator (*TIME1)() = TIME;
Indicator (*TIME2)(const KData&) = TIME;

Indicator (*YEAR1)() = YEAR;
Indicator (*YEAR2)(const KData&) = YEAR;

Indicator (*MONTH1)() = MONTH;
Indicator (*MONTH2)(const KData&) = MONTH;

Indicator (*WEEK1)() = WEEK;
Indicator (*WEEK2)(const KData&) = WEEK;

Indicator (*DAY1)() = DAY;
Indicator (*DAY2)(const KData&) = DAY;

Indicator (*HOUR1)() = HOUR;
Indicator (*HOUR2)(const KData&) = HOUR;

Indicator (*MINUTE1)() = MINUTE;
Indicator (*MINUTE2)(const KData&) = MINUTE;

// With too many overload options, python cannot load them
Indicator (*AMA_1)(int, int, int) = AMA;
Indicator (*AMA_2)(const IndParam&, const IndParam&, const IndParam&) = AMA;
Indicator (*AMA_3)(const Indicator&, int, int, int) = AMA;
Indicator (*AMA_4)(const Indicator&, const IndParam&, const IndParam&, const IndParam&) = AMA;
Indicator (*AMA_5)(const Indicator&, const Indicator&, const Indicator&, const Indicator&) = AMA;

Indicator (*DIFF_1)(int) = DIFF;
Indicator (*DIFF_2)(const Indicator&, int) = DIFF;

Indicator (*MA_1)(int) = MA;
Indicator (*MA_2)(const IndParam&) = MA;
Indicator (*MA_3)(const Indicator&, const IndParam&) = MA;
Indicator (*MA_4)(const Indicator&, const Indicator&) = MA;
Indicator (*MA_5)(const Indicator&, int) = MA;

Indicator (*WMA_1)(int) = WMA;
Indicator (*WMA_2)(const IndParam&) = WMA;
Indicator (*WMA_3)(const Indicator&, const IndParam&) = WMA;
Indicator (*WMA_4)(const Indicator&, const Indicator&) = WMA;
Indicator (*WMA_5)(const Indicator&, int) = WMA;

Indicator (*SMA_1)(int, double) = SMA;
Indicator (*SMA_2)(int, const IndParam&) = SMA;
Indicator (*SMA_3)(const IndParam&, double) = SMA;
Indicator (*SMA_4)(const IndParam&, const IndParam&) = SMA;
Indicator (*SMA_5)(const Indicator&, int, double) = SMA;
Indicator (*SMA_6)(const Indicator&, int, const IndParam&) = SMA;
Indicator (*SMA_7)(const Indicator&, const IndParam&, double) = SMA;
Indicator (*SMA_8)(const Indicator&, const IndParam&, const IndParam&) = SMA;
Indicator (*SMA_9)(const Indicator&, int, const Indicator&) = SMA;
Indicator (*SMA_10)(const Indicator&, const Indicator&, double) = SMA;
Indicator (*SMA_11)(const Indicator&, const Indicator&, const Indicator&) = SMA;

Indicator (*EMA_1)(int) = EMA;
Indicator (*EMA_2)(const IndParam&) = EMA;
Indicator (*EMA_3)(const Indicator&, const IndParam&) = EMA;
Indicator (*EMA_4)(const Indicator&, const Indicator&) = EMA;
Indicator (*EMA_5)(const Indicator&, int) = EMA;

Indicator (*MACD_1)(int, int, int) = MACD;
Indicator (*MACD_2)(const IndParam&, const IndParam&, const IndParam&) = MACD;
Indicator (*MACD_3)(const Indicator&, int, int, int) = MACD;
Indicator (*MACD_4)(const Indicator&, const IndParam&, const IndParam&, const IndParam&) = MACD;
Indicator (*MACD_5)(const Indicator&, const Indicator&, const Indicator&, const Indicator&) = MACD;

Indicator (*REF_1)(int) = REF;
Indicator (*REF_2)(const IndParam&) = REF;
Indicator (*REF_3)(const Indicator&, const IndParam&) = REF;
Indicator (*REF_4)(const Indicator&, const Indicator&) = REF;
Indicator (*REF_5)(const Indicator&, int) = REF;

Indicator (*SAFTYLOSS_1)(int n1, int n2, double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_2)(const IndParam& n1, const IndParam& n2, double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_3)(const IndParam& n1, const IndParam& n2, const IndParam& p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_4)(const Indicator&, int n1, int n2, double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_5)(const Indicator&, const IndParam& n1, const IndParam& n2,
                         double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_6)(const Indicator&, const IndParam& n1, const IndParam& n2,
                         const IndParam& p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_7)(const Indicator&, const Indicator& n1, const Indicator& n2,
                         double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_8)(const Indicator&, const Indicator& n1, const Indicator& n2,
                         const Indicator& p) = SAFTYLOSS;

Indicator (*STDEV_1)(int) = STDEV;
Indicator (*STDEV_2)(const IndParam&) = STDEV;
Indicator (*STDEV_3)(const Indicator&, const IndParam&) = STDEV;
Indicator (*STDEV_4)(const Indicator&, const Indicator&) = STDEV;
Indicator (*STDEV_5)(const Indicator&, int) = STDEV;

Indicator (*STDP_1)(int) = STDP;
Indicator (*STDP_2)(const IndParam&) = STDP;
Indicator (*STDP_3)(const Indicator&, const IndParam&) = STDP;
Indicator (*STDP_4)(const Indicator&, const Indicator&) = STDP;
Indicator (*STDP_5)(const Indicator&, int) = STDP;

Indicator (*HHV_1)(int) = HHV;
Indicator (*HHV_2)(const IndParam&) = HHV;
Indicator (*HHV_3)(const Indicator&, const Indicator&) = HHV;
Indicator (*HHV_4)(const Indicator&, const IndParam&) = HHV;
Indicator (*HHV_5)(const Indicator&, int) = HHV;

Indicator (*LLV_1)(int) = LLV;
Indicator (*LLV_2)(const IndParam&) = LLV;
Indicator (*LLV_3)(const Indicator&, const IndParam&) = LLV;
Indicator (*LLV_4)(const Indicator&, const Indicator&) = LLV;
Indicator (*LLV_5)(const Indicator&, int) = LLV;

Indicator (*VIGOR_1)(const KData&, int) = VIGOR;
Indicator (*VIGOR_2)(int) = VIGOR;

Indicator (*CVAL_1)(double, size_t) = CVAL;
Indicator (*CVAL_2)(const Indicator&, double, int) = CVAL;

Indicator (*LIUTONGPAN_1)() = LIUTONGPAN;
Indicator (*LIUTONGPAN_2)(const KData&) = LIUTONGPAN;

Indicator (*HSL_1)() = HSL;
Indicator (*HSL_2)(const KData&) = HSL;

Indicator (*IF_1)(const Indicator&, const Indicator&, const Indicator&) = IF;
Indicator (*IF_2)(const Indicator&, Indicator::value_t, const Indicator&) = IF;
Indicator (*IF_3)(const Indicator&, const Indicator&, Indicator::value_t) = IF;
Indicator (*IF_4)(const Indicator&, Indicator::value_t, Indicator::value_t) = IF;

Indicator (*COUNT_1)(int) = COUNT;
Indicator (*COUNT_2)(const IndParam&) = COUNT;
Indicator (*COUNT_3)(const Indicator&, const Indicator&) = COUNT;
Indicator (*COUNT_4)(const Indicator&, const IndParam&) = COUNT;
Indicator (*COUNT_5)(const Indicator&, int) = COUNT;

Indicator (*SUM_1)(int) = SUM;
Indicator (*SUM_2)(const IndParam&) = SUM;
Indicator (*SUM_3)(const Indicator&, const IndParam&) = SUM;
Indicator (*SUM_4)(const Indicator&, const Indicator&) = SUM;
Indicator (*SUM_5)(const Indicator&, int) = SUM;

Indicator (*ABS_1)() = ABS;
Indicator (*ABS_2)(Indicator::value_t) = ABS;
Indicator (*ABS_3)(const Indicator&) = ABS;

Indicator (*NOT_1)() = NOT;
Indicator (*NOT_2)(const Indicator&) = NOT;

Indicator (*SGN_1)() = SGN;
Indicator (*SGN_2)(Indicator::value_t) = SGN;
Indicator (*SGN_3)(const Indicator&) = SGN;

Indicator (*EXP_1)() = EXP;
Indicator (*EXP_2)(Indicator::value_t) = EXP;
Indicator (*EXP_3)(const Indicator&) = EXP;

Indicator (*MAX_1)(const Indicator&, const Indicator&) = MAX;
Indicator (*MAX_2)(const Indicator&, Indicator::value_t) = MAX;
Indicator (*MAX_3)(Indicator::value_t, const Indicator&) = MAX;

Indicator (*MIN_1)(const Indicator&, const Indicator&) = MIN;
Indicator (*MIN_2)(const Indicator&, Indicator::value_t) = MIN;
Indicator (*MIN_3)(Indicator::value_t, const Indicator&) = MIN;

Indicator (*BETWEEN_1)(const Indicator&, const Indicator&, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_2)(const Indicator&, const Indicator&, Indicator::value_t) = BETWEEN;
Indicator (*BETWEEN_3)(const Indicator&, Indicator::value_t, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_4)(const Indicator&, Indicator::value_t, Indicator::value_t) = BETWEEN;
Indicator (*BETWEEN_5)(Indicator::value_t, const Indicator&, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_6)(Indicator::value_t, const Indicator&, Indicator::value_t) = BETWEEN;
Indicator (*BETWEEN_7)(Indicator::value_t, Indicator::value_t, const Indicator&) = BETWEEN;
Indicator (*BETWEEN_8)(Indicator::value_t, Indicator::value_t, Indicator::value_t) = BETWEEN;

Indicator (*LN_1)() = LN;
Indicator (*LN_2)(Indicator::value_t) = LN;
Indicator (*LN_3)(const Indicator&) = LN;

Indicator (*LOG_1)() = LOG;
Indicator (*LOG_2)(Indicator::value_t) = LOG;
Indicator (*LOG_3)(const Indicator&) = LOG;

Indicator (*HHVBARS_1)(int) = HHVBARS;
Indicator (*HHVBARS_2)(const IndParam&) = HHVBARS;
Indicator (*HHVBARS_3)(const Indicator&, const IndParam&) = HHVBARS;
Indicator (*HHVBARS_4)(const Indicator&, const Indicator&) = HHVBARS;
Indicator (*HHVBARS_5)(const Indicator&, int) = HHVBARS;

Indicator (*LLVBARS_1)(int) = LLVBARS;
Indicator (*LLVBARS_2)(const IndParam&) = LLVBARS;
Indicator (*LLVBARS_3)(const Indicator&, const IndParam&) = LLVBARS;
Indicator (*LLVBARS_4)(const Indicator&, const Indicator&) = LLVBARS;
Indicator (*LLVBARS_5)(const Indicator&, int) = LLVBARS;

Indicator (*POW_1)(int) = POW;
Indicator (*POW_2)(const IndParam&) = POW;
Indicator (*POW_3)(const Indicator&, int) = POW;
Indicator (*POW_4)(const Indicator&, const IndParam&) = POW;
Indicator (*POW_5)(const Indicator&, const Indicator&) = POW;
Indicator (*POW_6)(Indicator::value_t, int) = POW;

Indicator (*SIGNED_POWER_1)(int) = SIGNED_POWER;
Indicator (*SIGNED_POWER_2)(const IndParam&) = SIGNED_POWER;
Indicator (*SIGNED_POWER_3)(const Indicator&, int) = SIGNED_POWER;
Indicator (*SIGNED_POWER_4)(const Indicator&, const IndParam&) = SIGNED_POWER;
Indicator (*SIGNED_POWER_5)(const Indicator&, const Indicator&) = SIGNED_POWER;
Indicator (*SIGNED_POWER_6)(Indicator::value_t, int) = SIGNED_POWER;

Indicator (*SQRT_1)() = SQRT;
Indicator (*SQRT_2)(const Indicator&) = SQRT;
Indicator (*SQRT_3)(Indicator::value_t) = SQRT;

Indicator (*ROUND_1)(int) = ROUND;
Indicator (*ROUND_2)(const Indicator&, int) = ROUND;
Indicator (*ROUND_3)(Indicator::value_t, int) = ROUND;

Indicator (*ROUNDUP_1)(int) = ROUNDUP;
Indicator (*ROUNDUP_2)(const Indicator&, int) = ROUNDUP;
Indicator (*ROUNDUP_3)(Indicator::value_t, int) = ROUNDUP;

Indicator (*ROUNDDOWN_1)(int) = ROUNDDOWN;
Indicator (*ROUNDDOWN_2)(const Indicator&, int) = ROUNDDOWN;
Indicator (*ROUNDDOWN_3)(Indicator::value_t, int) = ROUNDDOWN;

Indicator (*FLOOR_1)() = FLOOR;
Indicator (*FLOOR_2)(const Indicator&) = FLOOR;
Indicator (*FLOOR_3)(Indicator::value_t) = FLOOR;

Indicator (*CEILING_1)() = CEILING;
Indicator (*CEILING_2)(const Indicator&) = CEILING;
Indicator (*CEILING_3)(Indicator::value_t) = CEILING;

Indicator (*INTPART_1)() = INTPART;
Indicator (*INTPART_2)(const Indicator&) = INTPART;
Indicator (*INTPART_3)(Indicator::value_t) = INTPART;

Indicator (*EXIST_1)(int) = EXIST;
Indicator (*EXIST_2)(const IndParam&) = EXIST;
Indicator (*EXIST_3)(const Indicator&, const IndParam&) = EXIST;
Indicator (*EXIST_4)(const Indicator&, const Indicator&) = EXIST;
Indicator (*EXIST_5)(const Indicator&, int) = EXIST;

Indicator (*EVERY_1)(int) = EVERY;
Indicator (*EVERY_2)(const IndParam&) = EVERY;
Indicator (*EVERY_3)(const Indicator&, const IndParam&) = EVERY;
Indicator (*EVERY_4)(const Indicator&, const Indicator&) = EVERY;
Indicator (*EVERY_5)(const Indicator&, int) = EVERY;

Indicator (*LAST_1)(int, int) = LAST;
Indicator (*LAST_2)(int, const IndParam&) = LAST;
Indicator (*LAST_3)(const IndParam&, int) = LAST;
Indicator (*LAST_4)(const IndParam&, const IndParam&) = LAST;
Indicator (*LAST_5)(const Indicator&, int, int) = LAST;
Indicator (*LAST_6)(const Indicator&, int, const IndParam&) = LAST;
Indicator (*LAST_7)(const Indicator&, const IndParam&, int) = LAST;
Indicator (*LAST_8)(const Indicator&, const IndParam&, const IndParam&) = LAST;
Indicator (*LAST_9)(const Indicator&, int, const Indicator&) = LAST;
Indicator (*LAST_10)(const Indicator&, const Indicator&, int) = LAST;
Indicator (*LAST_11)(const Indicator&, const Indicator&, const Indicator&) = LAST;

Indicator (*SIN_1)() = SIN;
Indicator (*SIN_2)(const Indicator&) = SIN;
Indicator (*SIN_3)(Indicator::value_t) = SIN;

Indicator (*ASIN_1)() = ASIN;
Indicator (*ASIN_2)(const Indicator&) = ASIN;
Indicator (*ASIN_3)(Indicator::value_t) = ASIN;

Indicator (*COS_1)() = COS;
Indicator (*COS_2)(const Indicator&) = COS;
Indicator (*COS_3)(Indicator::value_t) = COS;

Indicator (*ACOS_1)() = ACOS;
Indicator (*ACOS_2)(const Indicator&) = ACOS;
Indicator (*ACOS_3)(Indicator::value_t) = ACOS;

Indicator (*TAN_1)() = TAN;
Indicator (*TAN_2)(const Indicator&) = TAN;
Indicator (*TAN_3)(Indicator::value_t) = TAN;

Indicator (*ATAN_1)() = ATAN;
Indicator (*ATAN_2)(const Indicator&) = ATAN;
Indicator (*ATAN_3)(Indicator::value_t) = ATAN;

Indicator (*REVERSE_1)() = REVERSE;
Indicator (*REVERSE_2)(const Indicator&) = REVERSE;
Indicator (*REVERSE_3)(Indicator::value_t) = REVERSE;

Indicator (*MOD_1)(const Indicator&, const Indicator&) = MOD;
Indicator (*MOD_2)(const Indicator&, Indicator::value_t) = MOD;
Indicator (*MOD_3)(Indicator::value_t, const Indicator&) = MOD;
Indicator (*MOD_4)(Indicator::value_t, Indicator::value_t) = MOD;

Indicator (*VAR_1)(int) = VAR;
Indicator (*VAR_2)(const IndParam&) = VAR;
Indicator (*VAR_3)(const Indicator&, const IndParam&) = VAR;
Indicator (*VAR_4)(const Indicator&, const Indicator&) = VAR;
Indicator (*VAR_5)(const Indicator&, int) = VAR;

Indicator (*VARP_1)(int) = VARP;
Indicator (*VARP_2)(const IndParam&) = VARP;
Indicator (*VARP_3)(const Indicator&, const IndParam&) = VARP;
Indicator (*VARP_4)(const Indicator&, const Indicator&) = VARP;
Indicator (*VARP_5)(const Indicator&, int) = VARP;

Indicator (*CROSS_1)(const Indicator&, const Indicator&) = CROSS;
Indicator (*CROSS_2)(const Indicator&, Indicator::value_t) = CROSS;
Indicator (*CROSS_3)(Indicator::value_t, const Indicator&) = CROSS;
Indicator (*CROSS_4)(Indicator::value_t, Indicator::value_t) = CROSS;

Indicator (*LONGCROSS_1)(const Indicator&, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_2)(const Indicator&, const Indicator&, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_3)(const Indicator&, Indicator::value_t, int) = LONGCROSS;
Indicator (*LONGCROSS_4)(const Indicator&, Indicator::value_t, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_5)(Indicator::value_t, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_6)(Indicator::value_t, const Indicator&, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_7)(Indicator::value_t, Indicator::value_t, int) = LONGCROSS;
Indicator (*LONGCROSS_8)(Indicator::value_t, Indicator::value_t, const Indicator&) = LONGCROSS;

Indicator (*FILTER_1)(int) = FILTER;
Indicator (*FILTER_2)(const IndParam&) = FILTER;
Indicator (*FILTER_3)(const Indicator&, const IndParam&) = FILTER;
Indicator (*FILTER_4)(const Indicator&, const Indicator&) = FILTER;
Indicator (*FILTER_5)(const Indicator&, int) = FILTER;

Indicator (*BARSSINCE_1)() = BARSSINCE;
Indicator (*BARSSINCE_2)(const Indicator&) = BARSSINCE;
Indicator (*BARSSINCE_3)(Indicator::value_t) = BARSSINCE;

Indicator (*BARSLAST_1)() = BARSLAST;
Indicator (*BARSLAST_2)(const Indicator&) = BARSLAST;
Indicator (*BARSLAST_3)(Indicator::value_t) = BARSLAST;

Indicator (*BARSLASTS_1)(int) = BARSLASTS;
Indicator (*BARSLASTS_2)(const IndParam&) = BARSLASTS;
Indicator (*BARSLASTS_3)(const Indicator&, int) = BARSLASTS;
Indicator (*BARSLASTS_4)(const Indicator&, const IndParam&) = BARSLASTS;
Indicator (*BARSLASTS_5)(const Indicator&, const Indicator&) = BARSLASTS;
Indicator (*BARSLASTS_6)(Indicator::value_t, int) = BARSLASTS;
Indicator (*BARSLASTS_7)(Indicator::value_t, const IndParam&) = BARSLASTS;
Indicator (*BARSLASTS_8)(Indicator::value_t, const Indicator&) = BARSLASTS;

Indicator (*SUMBARS_1)(double) = SUMBARS;
Indicator (*SUMBARS_2)(const IndParam&) = SUMBARS;
Indicator (*SUMBARS_3)(const Indicator&, const IndParam&) = SUMBARS;
Indicator (*SUMBARS_4)(const Indicator&, const Indicator&) = SUMBARS;
Indicator (*SUMBARS_5)(const Indicator&, double) = SUMBARS;

Indicator (*BARSCOUNT_1)() = BARSCOUNT;
Indicator (*BARSCOUNT_2)(const Indicator&) = BARSCOUNT;

Indicator (*BACKSET_1)(int) = BACKSET;
Indicator (*BACKSET_2)(const IndParam&) = BACKSET;
Indicator (*BACKSET_3)(const Indicator&, const IndParam&) = BACKSET;
Indicator (*BACKSET_4)(const Indicator&, const Indicator&) = BACKSET;
Indicator (*BACKSET_5)(const Indicator&, int) = BACKSET;

Indicator (*TIMELINE_1)() = TIMELINE;
Indicator (*TIMELINE_2)(const KData&) = TIMELINE;

Indicator (*TIMELINEVOL_1)() = TIMELINEVOL;
Indicator (*TIMELINEVOL_2)(const KData&) = TIMELINEVOL;

Indicator (*DEVSQ_1)(int) = DEVSQ;
Indicator (*DEVSQ_2)(const IndParam&) = DEVSQ;
Indicator (*DEVSQ_3)(const Indicator&, const Indicator&) = DEVSQ;
Indicator (*DEVSQ_4)(const Indicator&, const IndParam&) = DEVSQ;
Indicator (*DEVSQ_5)(const Indicator&, int) = DEVSQ;

Indicator (*ROC_1)(int) = ROC;
Indicator (*ROC_2)(const IndParam&) = ROC;
Indicator (*ROC_3)(const Indicator&, const IndParam&) = ROC;
Indicator (*ROC_4)(const Indicator&, const Indicator&) = ROC;
Indicator (*ROC_5)(const Indicator&, int) = ROC;

Indicator (*ROCP_1)(int) = ROCP;
Indicator (*ROCP_2)(const IndParam&) = ROCP;
Indicator (*ROCP_3)(const Indicator&, const IndParam&) = ROCP;
Indicator (*ROCP_4)(const Indicator&, const Indicator&) = ROCP;
Indicator (*ROCP_5)(const Indicator&, int) = ROCP;

Indicator (*ROCR_1)(int) = ROCR;
Indicator (*ROCR_2)(const IndParam&) = ROCR;
Indicator (*ROCR_3)(const Indicator&, const IndParam&) = ROCR;
Indicator (*ROCR_4)(const Indicator&, const Indicator&) = ROCR;
Indicator (*ROCR_5)(const Indicator&, int) = ROCR;

Indicator (*ROCR100_1)(int) = ROCR100;
Indicator (*ROCR100_2)(const IndParam&) = ROCR100;
Indicator (*ROCR100_3)(const Indicator&, const IndParam&) = ROCR100;
Indicator (*ROCR100_4)(const Indicator&, const Indicator&) = ROCR100;
Indicator (*ROCR100_5)(const Indicator&, int) = ROCR100;

Indicator (*AD_1)() = AD;
Indicator (*AD_2)(const KData&) = AD;

Indicator (*COST_1)(double x) = COST;
Indicator (*COST_2)(const KData&, double x) = COST;

Indicator (*ALIGN_1)(const DatetimeList&, bool fill_null) = ALIGN;
Indicator (*ALIGN_2)(const Indicator&, const DatetimeList&, bool fill_null) = ALIGN;
Indicator (*ALIGN_3)(const Indicator&, const Indicator&, bool fill_null) = ALIGN;
Indicator (*ALIGN_4)(const Indicator&, const KData&, bool fill_null) = ALIGN;

Indicator (*DROPNA_1)() = DROPNA;
Indicator (*DROPNA_2)(const Indicator&) = DROPNA;

Indicator (*AVEDEV_1)(const Indicator&, int) = AVEDEV;
Indicator (*AVEDEV_2)(const Indicator&, const IndParam&) = AVEDEV;
Indicator (*AVEDEV_3)(const Indicator&, const Indicator&) = AVEDEV;

Indicator (*DOWNNDAY_1)(const Indicator&, int) = DOWNNDAY;
Indicator (*DOWNNDAY_2)(const Indicator&, const IndParam&) = DOWNNDAY;
Indicator (*DOWNNDAY_3)(const Indicator&, const Indicator&) = DOWNNDAY;

Indicator (*UPNDAY_1)(const Indicator&, int) = UPNDAY;
Indicator (*UPNDAY_2)(const Indicator&, const IndParam&) = UPNDAY;
Indicator (*UPNDAY_3)(const Indicator&, const Indicator&) = UPNDAY;

Indicator (*NDAY_1)(const Indicator&, const Indicator&, int) = NDAY;
Indicator (*NDAY_2)(const Indicator&, const Indicator&, const Indicator&) = NDAY;
Indicator (*NDAY_3)(const Indicator&, const Indicator&, const IndParam&) = NDAY;

Indicator (*SLICE_1)(const PriceList&, int64_t, int64_t) = SLICE;
Indicator (*SLICE_2)(int64_t, int64_t, int) = SLICE;
Indicator (*SLICE_3)(const Indicator&, int64_t, int64_t, int) = SLICE;

Indicator (*RSI_1)(int) = RSI;
Indicator (*RSI_2)(const Indicator&, int) = RSI;

Indicator (*SLOPE1)(int) = SLOPE;
Indicator (*SLOPE2)(const IndParam&) = SLOPE;
Indicator (*SLOPE3)(const Indicator&, int) = SLOPE;
Indicator (*SLOPE4)(const Indicator&, const IndParam&) = SLOPE;
Indicator (*SLOPE5)(const Indicator&, const Indicator&) = SLOPE;

Indicator (*ZHBOND10_1)(double) = ZHBOND10;
Indicator (*ZHBOND10_2)(const DatetimeList&, double) = ZHBOND10;
Indicator (*ZHBOND10_3)(const KData& k, double) = ZHBOND10;
Indicator (*ZHBOND10_4)(const Indicator&, double) = ZHBOND10;

Indicator (*CORR_1)(const Indicator&, int, bool) = CORR;
Indicator (*CORR_2)(const Indicator&, const Indicator&, int, bool) = CORR;

Indicator (*COV_1)(const Indicator&, int, bool) = COV;
Indicator (*COV_2)(const Indicator&, const Indicator&, int, bool) = COV;

Indicator (*BETA_1)(const Indicator&, int, bool) = BETA;
Indicator (*BETA_2)(const Indicator&, const Indicator&, int, bool) = BETA;

Indicator (*RSRS_BETA_1)(int) = RSRS_BETA;
Indicator (*RSRS_BETA_2)(const KData&, int) = RSRS_BETA;

Indicator (*RSRS_BULL_1)(int, int) = RSRS_BULL;
Indicator (*RSRS_BULL_2)(const KData&, int, int) = RSRS_BULL;

Indicator (*SPEARMAN_1)(const Indicator&, int, bool) = SPEARMAN;
Indicator (*SPEARMAN_2)(const Indicator&, const Indicator&, int, bool) = SPEARMAN;

Indicator (*SKEW_1)(int) = SKEW;
Indicator (*SKEW_2)(const IndParam&) = SKEW;
Indicator (*SKEW_3)(const Indicator&, int) = SKEW;
Indicator (*SKEW_4)(const Indicator&, const IndParam&) = SKEW;
Indicator (*SKEW_5)(const Indicator&, const Indicator&) = SKEW;

Indicator (*KURT_1)(int) = KURT;
Indicator (*KURT_2)(const IndParam&) = KURT;
Indicator (*KURT_3)(const Indicator&, int) = KURT;
Indicator (*KURT_4)(const Indicator&, const IndParam&) = KURT;
Indicator (*KURT_5)(const Indicator&, const Indicator&) = KURT;

Indicator (*ZSCORE_1)(bool, double, bool) = ZSCORE;
Indicator (*ZSCORE_2)(const Indicator&, bool, double, bool) = ZSCORE;

Indicator (*ADX_1)(int) = ADX;
Indicator (*ADX_2)(const KData&, int) = ADX;

Indicator (*ADX2_1)(int) = ADX2;
Indicator (*ADX2_2)(const KData&, int) = ADX2;

void export_Indicator_build_in(py::module& m) {
    m.def("C_KDATA", KDATA1);
    m.def("C_KDATA", KDATA3, R"(KDATA([data])

    Wrap the KData as an Indicator, used for the calculation of the other indicators

    :param data: the KData or an Indicator with 6 returned results (such as the Indicator generated by KDATA)
    :rtype: Indicator)");

    m.def("C_CLOSE", CLOSE1);
    m.def("C_CLOSE", CLOSE3, R"(CLOSE([data])

    Get the close price; wrap the close price of the KData as an Indicator

    :param data: the input data (KData or Indicator)
    :rtype: Indicator)");

    m.def("C_OPEN", OPEN1);
    m.def("C_OPEN", OPEN3, R"(OPEN([data])

    Get the open price; wrap the open price of the KData as an Indicator

    :param data: the input data (KData or Indicator) 
    :rtype: Indicator)");

    m.def("C_HIGH", HIGH1);
    m.def("C_HIGH", HIGH3, R"(HIGH([data])

    Get the high price; wrap the high price of the KData as an Indicator

    :param data: the input data (KData or Indicator) 
    :rtype: Indicator)");

    m.def("C_LOW", LOW1);
    m.def("C_LOW", LOW3, R"(LOW([data])

    Get the low price; wrap the low price of the KData as an Indicator

    :param data: the input data (KData or Indicator) 
    :rtype: Indicator)");

    m.def("C_AMO", AMO1);
    m.def("C_AMO", AMO3, R"(AMO([data])

    Get the amount; wrap the amount of the KData as an Indicator
    
    :param data: the input data (KData or Indicator)
    :rtype: Indicator)");

    m.def("C_VOL", VOL1);
    m.def("C_VOL", VOL3, R"(VOL([data])

    Get the volume; wrap the volume of the KData as an Indicator

    :param data: the input data (KData or Indicator)
    :rtype: Indicator)");

    m.def("KDATA_PART", KDATA_PART1, py::arg("data"), py::arg("kpart"));
    m.def("KDATA_PART", KDATA_PART3, py::arg("kpart"), R"(KDATA_PART([data, kpart])

    Select and return the indicator KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL by the string, e.g.: KDATA_PART("CLOSE") is equivalent to CLOSE()

    :param data: the input data (KData or Indicator) 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :rtype: Indicator)");

    m.def("RECOVER_FORWARD", RECOVER_FORWARD_1);
    m.def("RECOVER_FORWARD", RECOVER_FORWARD_2);
    m.def("RECOVER_FORWARD", RECOVER_FORWARD_3, R"(RECOVER_FORWARD([data])
    
    Perform the forward recovery on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator)");

    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_1);
    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_2);
    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_3, R"(RECOVER_BACKWARD([data])
    
    Perform the backward recovery on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator)");

    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_1);
    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_2);
    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_3, R"(RECOVER_EQUAL_FORWARD([data])
    
    Perform the equal-ratio forward recovery on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator)");

    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_1);
    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_2);
    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_3, R"(RECOVER_EQUAL_BACKWARD([data])
    
    Perform the equal-ratio backward recovery on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator)");

    m.def("DATE", DATE1);
    m.def("DATE", DATE2, R"(DATE([data])

    Get the year-month-day of this period since 1900. Usage: DATE, e.g. the function returns 1000101, which means January 1, 2000.

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("TIME", TIME1);
    m.def("TIME", TIME2, R"(TIME([data])

    Get the hour-minute-second of this period. Usage: TIME, the valid value range returned by the function is (000000-235959).

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("YEAR", YEAR1);
    m.def("YEAR", YEAR2, R"(YEAR([data])

    Get the year of this period.

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("MONTH", MONTH1);
    m.def("MONTH", MONTH2, R"(MONTH([data])

    Get the month of this period. Usage: MONTH, the valid value range returned by the function is (1-12).

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("WEEK", WEEK1);
    m.def("WEEK", WEEK2, R"(WEEK([data])

    Get the day of the week of this period. Usage: WEEK, the valid value range returned by the function is (0-6), and 0 means Sunday.

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("DAY", DAY1);
    m.def("DAY", DAY2, R"(DAY([data])

    Get the date of this period. Usage: DAY, the valid value range returned by the function is (1-31).

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("HOUR", HOUR1);
    m.def("HOUR", HOUR2, R"(HOUR([data])

    Get the number of the hours of this period. Usage: HOUR, the valid value range returned by the function is (0-23), and the value is 0 for the daily and the longer analysis periods.

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("MINUTE", MINUTE1);
    m.def("MINUTE", MINUTE2, R"(MINUTE([data])

    Get the number of the minutes of this period. Usage: MINUTE, the valid value range returned by the function is (0-59), and the value is 0 for the daily and the longer analysis periods.

    :param data: the input data KData
    :rtype: Indicator)");

    m.def("CONTEXT", py::overload_cast<bool, bool, bool>(hku::CONTEXT),
          py::arg("fill_null") = false, py::arg("use_self_ktype") = false,
          py::arg("use_self_recover_type") = false);
    m.def("CONTEXT", py::overload_cast<const Indicator&, bool, bool, bool>(hku::CONTEXT),
          py::arg("ind"), py::arg("fill_null") = false, py::arg("use_self_ktype") = false,
          py::arg("use_self_recover_type") = false, R"(CONTEXT(ind)
    
    The independent context. It uses the context that the ind comes with. When a new context is specified, it will not change the existing context.
    For example: ind = CLOSE(k1); when a new context ind = ind(k2) is specified, the close price of k2 is used. If you still want to use the close price of k1,
    you need to use ind = CONTEXT(CLOSE(k1)); at this time ind(k2) will still use the close price of k1.
    
    :param Indicator ind: the indicator object
    :param bool fill_null: during the date alignment, the missing dates are filled with the empty values; otherwise, they are filled with the previous values.
    :param bool use_self_ktype: use the KTYPE in its own independent context when calculating the formula
    :param bool use_self_recover_type: use the RECOVER_TYPE in its own independent context when calculating the formula
    :rtype: Indicator)");

    m.def("CONTEXT", py::overload_cast<const Indicator&, const Stock&, bool>(hku::CONTEXT),
          py::arg("ind"), py::arg("stock"), py::arg("fill_null") = false,
          R"(CONTEXT(ind, stock[, fill_null=False]) 
          
    Set the independent context of the indicator by specifying the stock, ignoring the context of the passed ind itself, and directly using the stock as the context

    :param Indicator ind: the indicator object
    :param Stock stock: the stock object
    :param bool fill_null: during the date alignment, the missing dates are filled with the empty values; otherwise, they are filled with the previous values.
    :rtype: Indicator)");

    m.def("CONTEXT_K", CONTEXT_K, R"(CONTEXT_K(ind)

    Get the indicator context. The Indicator::getContext() method gets the current context, but it cannot get the specified independent context for the CONTEXT independent context indicator; you need to use this method to get it

    Once this indicator participates in the calculation as a formula, its context may change, but its stock remains unchanged, and only the query range changes

    :param Indicator ind: the indicator object
    :rtype: KData)");

    m.def(
      "PRICELIST",
      [](const DatetimeList& dates, double value, int discard) {
          return PRICELIST(dates, value, discard);
      },
      py::arg("dates"), py::arg("value"), py::arg("discard") = 0);
    m.def(
      "PRICELIST",
      [](size_t size, double value, int discard) { return PRICELIST(size, value, discard); },
      py::arg("size"), py::arg("value"), py::arg("discard") = 0);
    m.def(
      "PRICELIST",
      [](const py::object& obj = py::none(), int discard = 0,
         const py::object& pyalign_dates = py::none()) {
          if (obj.is_none()) {
              return PRICELIST();
          } else if (py::isinstance<py::sequence>(obj)) {
              const auto& x = obj.cast<py::sequence>();
              auto values = python_list_to_vector<price_t>(x);
              if (pyalign_dates.is_none()) {
                  return PRICELIST(std::move(values), discard);
              } else {
                  py::sequence align_dates = pyalign_dates.cast<py::sequence>();
                  auto total = len(align_dates);
                  DatetimeList dates(total);
                  for (auto i = 0; i < total; ++i) {
                      dates[i] = pydatetime_to_Datetime(align_dates[i]);
                  }
                  return PRICELIST(std::move(values), std::move(dates), discard);
              }
          } else {
              HKU_THROW("Invalid input data type!");
          }
      },
      py::arg("data") = py::none(), py::arg("discard") = 0, py::arg("align_dates") = py::none(),
      R"(PRICELIST([data=None, discard=0, align_dates=None])
      
    Convert a python array (such as a list, a tuple or a numpy.array) to an Indicator object.
    
    :param sequence data: the input data
    :param int discard: how many data to discard at the front
    :param sequence align_dates: the aligned date list; if it is empty, no alignment is performed
    :rtype: Indicator)");

    m.def("SMA", SMA_1, py::arg("n") = 22, py::arg("m") = 2.0);
    m.def("SMA", SMA_2, py::arg("n"), py::arg("m"));
    m.def("SMA", SMA_3, py::arg("n"), py::arg("m") = 2.0);
    m.def("SMA", SMA_4, py::arg("n"), py::arg("m"));
    m.def("SMA", SMA_5, py::arg("data"), py::arg("n") = 22, py::arg("m") = 2.0);
    m.def("SMA", SMA_6, py::arg("data"), py::arg("n"), py::arg("m"));
    m.def("SMA", SMA_7, py::arg("data"), py::arg("n"), py::arg("m") = 2.0);
    m.def("SMA", SMA_8, py::arg("data"), py::arg("n"), py::arg("m"));
    m.def("SMA", SMA_9, py::arg("data"), py::arg("n"), py::arg("m"));
    m.def("SMA", SMA_10, py::arg("data"), py::arg("n"), py::arg("m") = 2.0);
    m.def("SMA", SMA_11, py::arg("data"), py::arg("n"), py::arg("m"), R"(SMA([data, n=22, m=2])

    Get the moving average

    Usage: if Y=SMA(X,N,M), then Y=[M*X+(N-M)*Y')/N, where Y' represents the Y value of the previous period

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :param float|Indicator|IndParam m: the coefficient
    :rtype: Indicator)");

    m.def("EMA", EMA_1, py::arg("n") = 22);
    m.def("EMA", EMA_2, py::arg("n"));
    m.def("EMA", EMA_3, py::arg("data"), py::arg("n"));
    m.def("EMA", EMA_4, py::arg("data"), py::arg("n"));
    m.def("EMA", EMA_5, py::arg("data"), py::arg("n") = 22, R"(EMA([data, n=22])

    The Exponential Moving Average

    :param data: the input data
    :param int|Indicator|IndParam n n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator)");

    m.def("MA", MA_1, py::arg("n") = 22);
    m.def("MA", MA_2, py::arg("n"));
    m.def("MA", MA_3, py::arg("data"), py::arg("n"));
    m.def("MA", MA_4, py::arg("data"), py::arg("n"));
    m.def("MA", MA_5, py::arg("data"), py::arg("n") = 22, R"(MA([data, n=22])

    The simple moving average

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("WMA", WMA_1, py::arg("n") = 22);
    m.def("WMA", WMA_2, py::arg("n"));
    m.def("WMA", WMA_3, py::arg("data"), py::arg("n"));
    m.def("WMA", WMA_4, py::arg("data"), py::arg("n"));
    m.def("WMA", WMA_5, py::arg("data"), py::arg("n") = 22, R"(WMA([data, n=22])

    The weighted moving average, with the algorithm: Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("AMA", AMA_1, py::arg("n") = 10, py::arg("fast_n") = 2, py::arg("slow_n") = 30);
    m.def("AMA", AMA_2, py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_4, py::arg("data"), py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_5, py::arg("data"), py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_3, py::arg("data"), py::arg("n") = 10, py::arg("fast_n") = 2,
          py::arg("slow_n") = 30,
          R"(AMA([data, n=10, fast_n=2, slow_n=30])

    The Perry J. Kaufman adaptive moving average [BOOK1]_

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the period window for calculating the average, which must be an integer greater than 2
    :param int|Indicator|IndParam fast_n: the corresponding fast period N
    :param int|Indicator|IndParam slow_n: the N value corresponding to the slow EMA line
    :rtype: Indicator

    * result(0): AMA
    * result(1): ER)");

    m.def("ATR", py::overload_cast<int>(ATR), py::arg("n") = 14);
    m.def("ATR", py::overload_cast<const KData&, int>(ATR), py::arg("kdata"), py::arg("n") = 14,
          R"(ATR([kdata, n=14])

    The Average True Range, the simple moving average of the true range TR

    :param KData kdata the source data to calculate
    :param int n: the period window for calculating the average, which must be an integer greater than 1
    :rtype: Indicator)");

    m.def("ADX", ADX_1, py::arg("n") = 14);
    m.def("ADX", ADX_2, py::arg("kdata"), py::arg("n") = 14,
          R"(ADX([kdata, n=14])

    The Average Directional Index

    ADX belongs to the trend strength indicators; it does not distinguish the up/down direction, but only judges whether there is a trend. It uses the Wilder original formula.

    :param KData kdata: the source data to calculate
    :param int n: the calculation period, defaulting to 14, which must be an integer greater than 1
    :rtype: an Indicator with three result sets

    * result(0): the ADX itself (the trend strength, with the value range 0~100)
    * result(1): +DI (the rising direction line, the bull force)
    * result(2): -DI (the falling direction line, the bear force)

    The judgment criteria:
    - ADX >= 25: there is a clear one-sided trend (both rising/falling are fine)
    - ADX < 25: no trend, a range oscillation
    - The greater the ADX value, the stronger the trend)");

    m.def("ADX2", ADX2_1, py::arg("n") = 14);
    m.def("ADX2", ADX2_2, py::arg("kdata"), py::arg("n") = 14,
          R"(ADX2([kdata, n=14])

    The Average Directional Index (ADX2) - using the EMA smoothing way

    ADX2 belongs to the trend strength indicators; it does not distinguish the up/down direction, but only judges whether there is a trend.
    The difference from the ADX is that it uses the EMA (the Exponential Moving Average) instead of the Wilder smoothing, being more sensitive to the trend changes.

    :param KData kdata: the source data to calculate
    :param int n: the calculation period, defaulting to 14, which must be an integer greater than 1
    :rtype: an Indicator with three result sets

    * result(0): the ADX2 itself (the trend strength, with the value range 0~100)
    * result(1): +DI (the rising direction line, the bull force)
    * result(2): -DI (the falling direction line, the bear force)

    The judgment criteria:
    - ADX2 >= 25: there is a clear one-sided trend (both rising/falling are fine)
    - ADX2 < 25: no trend, a range oscillation
    - The greater the ADX2 value, the stronger the trend)");

    m.def("MACD", MACD_1, py::arg("n1") = 12, py::arg("n2") = 26, py::arg("n3") = 9);
    m.def("MACD", MACD_2, py::arg("n1"), py::arg("n2"), py::arg("n3"));
    m.def("MACD", MACD_3, py::arg("data"), py::arg("n1") = 12, py::arg("n2") = 26,
          py::arg("n3") = 9);
    m.def("MACD", MACD_4, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("n3"));
    m.def("MACD", MACD_5, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("n3"),
          R"(MACD([data, n1=12, n2=26, n3=9])

    The Moving Average Convergence Divergence

    :param Indicator data: the input data
    :param int|Indicator|IndParam n1: the short-term EMA time window
    :param int|Indicator|IndParam n2: the long-term EMA time window
    :param int|Indicator|IndParam n3: the EMA smoothing time window of (the short-term EMA - the long-term EMA)
    :rtype: an Indicator with three result sets

    * result(0): MACD_BAR: the MACD bar, i.e. the MACD fast line - the MACD slow line
    * result(1): DIFF: the fast line, i.e. (the short-term EMA - the long-term EMA)
    * result(2): DEA: the slow line, i.e. the n3-period EMA smoothing of the fast line)");

    m.def("VIGOR", VIGOR_1, py::arg("kdata"), py::arg("n") = 2);
    m.def("VIGOR", VIGOR_2, py::arg("n") = 2, R"(VIGOR([kdata, n=2])

    The Alexander Elder force index [BOOK2]_

    The calculation formula: (the close price today - the close price yesterday) * the volume today

    :param KData kdata: the input data
    :param int n: the EMA smoothing window
    :rtype: Indicator)");

    m.def("SAFTYLOSS", SAFTYLOSS_1, py::arg("n1") = 10, py::arg("n2") = 3, py::arg("p") = 2.0);
    m.def("SAFTYLOSS", SAFTYLOSS_2, py::arg("n1"), py::arg("n2"), py::arg("p") = 2.0);
    m.def("SAFTYLOSS", SAFTYLOSS_3, py::arg("n1"), py::arg("n2"), py::arg("p"));
    m.def("SAFTYLOSS", SAFTYLOSS_4, py::arg("data"), py::arg("n1") = 10, py::arg("n2") = 3,
          py::arg("p") = 2.0);
    m.def("SAFTYLOSS", SAFTYLOSS_5, py::arg("data"), py::arg("n1"), py::arg("n2"),
          py::arg("p") = 2.0);
    m.def("SAFTYLOSS", SAFTYLOSS_6, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("p"));
    m.def("SAFTYLOSS", SAFTYLOSS_7, py::arg("data"), py::arg("n1"), py::arg("n2"),
          py::arg("p") = 2.0);
    m.def("SAFTYLOSS", SAFTYLOSS_8, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("p"),
          R"(SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    The Alexander Elder safety zone stop line, see [BOOK2]_

    The calculation description: within the lookback period (generally 10 to 20 days), sum all the lengths of the downward penetrations and divide by the number of the downward penetrations to get the average noise (i.e. the total length of all the lowest prices below the lowest price of the previous day within the lookback period divided by the number of the times), and subtract (the average noise of the previous day multiplied by a multiple) from the lowest price of today to get the stop line. To offset the fluctuation and ensure the upward movement of the stop line, take the highest value within the N days (generally 3 days) on the basis of the above result

    :param Indicator data: the input data
    :param int|Indicator|IndParam n1: the lookback time window for calculating the average noise
    :param int|Indicator|IndParam n2: take the highest value within the n2 days for the preliminary stop line
    :param float|Indicator|IndParam p: the noise coefficient
    :rtype: Indicator)");

    m.def("DIFF", DIFF_1, py::arg("n") = 1);
    m.def("DIFF", DIFF_2, py::arg("data"), py::arg("n") = 1, R"(DIFF([data, n=1])

    The difference indicator, i.e. data[i] - data[i-n]

    :param Indicator data: the input data
    :param int n: the difference period, defaulting to 1
    :rtype: Indicator)");

    m.def("REF", REF_1, py::arg("n"));
    m.def("REF", REF_2, py::arg("n"));
    m.def("REF", REF_3, py::arg("data"), py::arg("n"));
    m.def("REF", REF_4, py::arg("data"), py::arg("n"));
    m.def("REF", REF_5, py::arg("data"), py::arg("n"), R"(REF([data, n])

    The forward reference (i.e. shifting right), referencing the data several periods ago.

    Usage: REF(X,A) references the value of X A periods ago.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: reference the value n periods ago, i.e. shifting right by n positions
    :rtype: Indicator)");

    m.def("STDEV", STDEV_1, py::arg("n") = 10);
    m.def("STDEV", STDEV_2, py::arg("n"));
    m.def("STDEV", STDEV_3, py::arg("data"), py::arg("n"));
    m.def("STDEV", STDEV_4, py::arg("data"), py::arg("n"));
    m.def("STDEV", STDEV_5, py::arg("data"), py::arg("n") = 10, R"(STDEV([data, n=10])

    Calculate the sample standard deviation within the N periods

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("STDP", STDP_1, py::arg("n") = 10);
    m.def("STDP", STDP_2, py::arg("n"));
    m.def("STDP", STDP_3, py::arg("data"), py::arg("n"));
    m.def("STDP", STDP_4, py::arg("data"), py::arg("n"));
    m.def("STDP", STDP_5, py::arg("data"), py::arg("n") = 10, R"(STDP([data, n=10])

    The population standard deviation; STDP(X,N) is the N-day population standard deviation of X

    :param data: the input data
    :param int n: the time window
    :rtype: Indicator)");

    m.def("HHV", HHV_1, py::arg("n") = 20);
    m.def("HHV", HHV_2, py::arg("n"));
    m.def("HHV", HHV_3, py::arg("data"), py::arg("n"));
    m.def("HHV", HHV_4, py::arg("data"), py::arg("n"));
    m.def("HHV", HHV_5, py::arg("data"), py::arg("n") = 20, R"(HHV([data, n=20])

    The highest price within the N days; when N=0, start from the first valid value.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator)");

    m.def("LLV", LLV_1, py::arg("n") = 20);
    m.def("LLV", LLV_2, py::arg("n"));
    m.def("LLV", LLV_3, py::arg("data"), py::arg("n"));
    m.def("LLV", LLV_4, py::arg("data"), py::arg("n"));
    m.def("LLV", LLV_5, py::arg("data"), py::arg("n") = 20, R"(LLV([data, n=20])

    The lowest price within the N days; when N=0, start from the first valid value.

    :param data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator)");

    m.def("CVAL", CVAL_1, py::arg("value") = 0.0, py::arg("discard") = 0);
    m.def("CVAL", CVAL_2, py::arg("data"), py::arg("value") = 0.0, py::arg("discard") = 0,
          R"(CVAL([data, value=0.0, discard=0])

    data is an Indicator instance; create a constant indicator with the same length as data, whose value is value, and the discard length is the same as data

    :param Indicator data: the Indicator instance
    :param float value: the constant value
    :param int discard: the discard quantity
    :rtype: Indicator)");

    m.def("LIUTONGPAN", LIUTONGPAN_1);
    m.def("LIUTONGPAN", LIUTONGPAN_2, R"(LIUTONGPAN(kdata)

   Get the circulating share capital (unit: 10,000 shares), the same as CAPITAL

   :param KData kdata: the K-line data
   :rtype: Indicator)");

    m.def("ZONGGUBEN", py::overload_cast<>(ZONGGUBEN));
    m.def("ZONGGUBEN", py::overload_cast<const KData&>(ZONGGUBEN), R"(ZONGGUBEN(kdata)

   Get the total share capital (unit: 10,000 shares)

   :param KData kdata: the K-line data
   :rtype: Indicator)");

    m.def("HSL", HSL_1);
    m.def("HSL", HSL_2, R"(HSL(kdata)

    Get the turnover rate; multiplying by 100 gives the percentage, equal to VOL(k) / CAPITAL(k) * 0.01

    :param KData kdata: the K-line data
    :rtype: Indicator)");

    m.def("WEAVE", [](const py::sequence& seq) {
        size_t total = len(seq);
        HKU_CHECK(total >= 2 && total <= 6, "{}", htr("WEAVE: total must be 2 to 6"));
        Indicator ind1 = seq[0].cast<Indicator>();
        Indicator ind2 = seq[1].cast<Indicator>();
        Indicator tmp = WEAVE(ind1, ind2);
        for (size_t i = 2; i < total; i++) {
            tmp = WEAVE(tmp, seq[i].cast<const Indicator&>());
        }
        return tmp;
    });
    m.def("WEAVE", [](const Indicator& ind1, const Indicator& ind2) { return WEAVE(ind1, ind2); });
    m.def("WEAVE", [](const Indicator& ind1, const Indicator& ind2, const Indicator& ind3) {
        return WEAVE(ind1, ind2, ind3);
    });
    m.def("WEAVE", [](const Indicator& ind1, const Indicator& ind2, const Indicator& ind3,
                      const Indicator& ind4) { return WEAVE(ind1, ind2, ind3, ind4); });
    m.def("WEAVE", [](const Indicator& ind1, const Indicator& ind2, const Indicator& ind3,
                      const Indicator& ind4,
                      const Indicator& ind5) { return WEAVE(ind1, ind2, ind3, ind4, ind5); });
    m.def(
      "WEAVE",
      [](const Indicator& ind1, const Indicator& ind2, const Indicator& ind3, const Indicator& ind4,
         const Indicator& ind5,
         const Indicator& ind6) { return WEAVE(ind1, ind2, ind3, ind4, ind5, ind6); },
      R"(WEAVE(ind1, ind2[, ind3, ind4, ind5, ind6])

    Combine the results of at most 6 Indicators together in one Indicator. E.g. ind = WEAVE(ind1, ind2), then ind contains the multiple results at this time, stored in the order of ind1 and ind2.
    
    :param Indicator ind1: the indicator 1
    :param Indicator ind2: the indicator 2
    :param Indicator ind3: the indicator 3, which can be omitted
    :param Indicator ind4: the indicator 4, which can be omitted
    :param Indicator ind5: the indicator 5, which can be omitted
    :param Indicator ind6: the indicator 6, which can be omitted
    :rtype: Indicator)");

    m.def("CORR", CORR_1, py::arg("ref_ind"), py::arg("n") = 10, py::arg("fill_null") = true);
    m.def("CORR", CORR_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 10,
          py::arg("fill_null") = true,
          R"(CORR(ind, ref_ind[, n=10, fill_null=True])

    Calculate the correlation coefficient of ind and ref_ind. There are two results in the return; the first is the correlation coefficient, and the second is the covariance.
    Equivalent to CORR(ref_ind, n)(ind).

    :param Indicator ind: the indicator1
    :param Indicator ref_ind: the indicator 2
    :param int n: calculate the correlation coefficient of the data between the two inds by the specified length n. If it is 0, use the length of the input ind.
    :param bool fill_null: fill the missing dates with the nan values during the date alignment
    :rtype: Indicator)");

    m.def("COV", COV_1, py::arg("ref_ind"), py::arg("n") = 10, py::arg("fill_null") = true);
    m.def("COV", COV_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 10,
          py::arg("fill_null") = true,
          R"(COV(ind, ref_ind[, n=10, fill_null=True])

    Calculate the sample covariance of ind and ref_ind.
    Equivalent to COV(ref_ind, n)(ind).

    :param Indicator ind: the indicator1
    :param Indicator ref_ind: the indicator 2
    :param int n: calculate the covariance of the data between the two inds by the specified length n. If it is 0, use the length of the input ind.
    :param bool fill_null: fill the missing dates with the nan values during the date alignment
    :rtype: Indicator)");

    m.def("BETA", BETA_1, py::arg("ref_ind"), py::arg("n") = 10, py::arg("fill_null") = true);
    m.def("BETA", BETA_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 10,
          py::arg("fill_null") = true,
          R"(BETA(ind, ref_ind[, n=10, fill_null=True])

    Calculate the Beta coefficient, e.g. measuring the sensitivity between the asset return and the market return.
    Beta = Cov(stock_return, market_return) / Var(market_return)
    Equivalent to BETA(ref_ind, n)(ind).

    .. note:: BETA itself does not convert the input data to the return rate (pct_change);
              the input indicator should be the already calculated return rate data.

    :param Indicator ind: the input indicator, such as the stock return indicator
    :param Indicator ref_ind: the reference indicator, such as the market return indicator
    :param int n: the rolling window size (greater than 2 or equal to 0). If it is 0, use the length of the input ind.
    :param bool fill_null: fill the missing dates with the nan values during the date alignment
    :rtype: Indicator)");

    m.def("RSRS_BETA", RSRS_BETA_1, py::arg("n") = 20);
    m.def("RSRS_BETA", RSRS_BETA_2, py::arg("kdata"), py::arg("n") = 20,
          R"(RSRS_BETA([kdata, n=20])

    The original RSRS (the underlying β) indicator, based on the rolling N-day OLS regression.

    Each K-line contributes a coordinate point (Low[i], High[i]), and the N points within the rolling window are used for the OLS regression.
    The formula: High = α + β · Low

    β is the most original RSRS slope, representing the strength of the support and the resistance.
    The defect: the β center fluctuates greatly in the different market ranges, and it cannot be compared directly across the time periods.

    :param KData kdata: the K-line data
    :param int n: the rolling window, defaulting to 20
    :rtype: Indicator)");

    m.def("RSRS_BULL", RSRS_BULL_1, py::arg("n") = 20, py::arg("m") = 60);
    m.def("RSRS_BULL", RSRS_BULL_2, py::arg("kdata"), py::arg("n") = 20, py::arg("m") = 60,
          R"(RSRS_BULL([kdata, n=20, m=60])

    The RSRS right-skewed standard score indicator (level 4), based on the advanced version of the RSRS corrected by the Everbright research report.

    On the basis of the corrected standard score (Z × R²), it is multiplied by the original β once again, amplifying the scores in the strong bull zone and compressing the scores in the weak bear zone, specifically for the long timing.

    **The calculation layers**:

    1. **Level 1 (β)**: calculate the β value with the rolling N-day OLS regression, with the formula: High = α + β · Low
    2. **Level 2 (Z)**: the rolling M-day Z-score standardization, solving the problem of the β center drifting in the different stages
    3. **Level 3 (the corrected standard score)**: Z × R², where R² is the regression goodness of fit, filtering the noise with the poor fitting
    4. **Level 4 (the right-skewed correction)**: Z × R² × β, amplifying the strong bull zone

    **The returned results**:

    * result(0): the level 4 corrected value (the right-skewed correction = Z × R² × β)
    * result(1): the β value (the regression slope)
    * result(2): the R² value (the regression goodness of fit)
    * result(3): the Z value (the Z-score standardized value)

    :param KData kdata: the K-line data
    :param int n: the regression window, defaulting to 20
    :param int m: the Z-score window, defaulting to 60
    :rtype: Indicator)");

    m.def("IF", IF_1);
    m.def("IF", IF_2);
    m.def("IF", IF_3);
    m.def("IF", IF_4, R"(IF(x, a, b)

    The condition function, getting the different values according to the condition.

    Usage: IF(X,A,B); if X is not 0, return A, otherwise return B

    For example: IF(CLOSE>OPEN,HIGH,LOW) means that if this period closes bullish, return the highest value, otherwise return the lowest value

    :param Indicator x: the condition indicator
    :param Indicator a: the candidate indicator a
    :param Indicator b: the candidate indicator b
    :rtype: Indicator)");

    m.def("COUNT", COUNT_1, py::arg("n") = 20);
    m.def("COUNT", COUNT_2, py::arg("n"));
    m.def("COUNT", COUNT_3, py::arg("data"), py::arg("n"));
    m.def("COUNT", COUNT_4, py::arg("data"), py::arg("n"));
    m.def("COUNT", COUNT_5, py::arg("data"), py::arg("n") = 20, R"(COUNT([data, n=20])

    Count the number of the periods satisfying the condition.

    Usage: COUNT(X,N), counting the number of the periods satisfying the condition X within the N periods; if N=0, start from the first valid value.

    For example: COUNT(CLOSE>OPEN,20) means counting the number of the periods closing bullish within the 20 periods

    :param Indicator data: the condition
    :param int|Indicator|IndParam n: the periods
    :rtype: Indicator)");

    m.def("SUM", SUM_1, py::arg("n") = 20);
    m.def("SUM", SUM_2, py::arg("n"));
    m.def("SUM", SUM_3, py::arg("data"), py::arg("n"));
    m.def("SUM", SUM_4, py::arg("data"), py::arg("n"));
    m.def("SUM", SUM_5, py::arg("data"), py::arg("n") = 20, R"(SUM([data, n=20])

    Get the total sum. SUM(X,N), counting the total sum of X within the N periods; when N=0, start from the first valid value.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("ABS", ABS_1);
    m.def("ABS", ABS_2);
    m.def("ABS", ABS_3, R"(ABS([data])

    Get the absolute value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("NOT", NOT_1);
    m.def("NOT", NOT_2, R"(NOT([data])

    Get the logical negation. NOT(X) returns the negation of X, i.e. it returns 1 when X=0, otherwise 0.

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("SGN", SGN_1);
    m.def("SGN", SGN_2);
    m.def("SGN", SGN_3, R"(SGN([data])

    Get the sign value; SGN(X) returns 1, 0, -1 when X>0, X=0, X<0 respectively.

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("EXP", EXP_1);
    m.def("EXP", EXP_2);
    m.def("EXP", EXP_3, R"(EXP([data])

    EXP(X) is e to the power of X

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("MAX", MAX_1);
    m.def("MAX", MAX_2);
    m.def("MAX", MAX_3, R"(MAX(ind1, ind2)

    Get the maximum value; MAX(A,B) returns the greater value of A and B.

    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator)");

    m.def("MIN", MIN_1);
    m.def("MIN", MIN_2);
    m.def("MIN", MIN_3, R"(MIN(ind1, ind2)

    Get the minimum value; MIN(A,B) returns the smaller value of A and B.

    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator)");

    m.def("BETWEEN", BETWEEN_1);
    m.def("BETWEEN", BETWEEN_2);
    m.def("BETWEEN", BETWEEN_3);
    m.def("BETWEEN", BETWEEN_4);
    m.def("BETWEEN", BETWEEN_5);
    m.def("BETWEEN", BETWEEN_6);
    m.def("BETWEEN", BETWEEN_7);
    m.def("BETWEEN", BETWEEN_8, R"(BETWEEN(a, b, c)

    Between (between two numbers)

    Usage: BETWEEN(A,B,C) means that it returns 1 when A is between B and C, otherwise 0

    For example: BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5)) means that the close price is between the 5-day MA and the 10-day MA

    :param Indicator a: A
    :param Indicator b: B
    :param Indicator c: C
    :rtype: Indicator)");

    m.def("LN", LN_1);
    m.def("LN", LN_2);
    m.def("LN", LN_3, R"(LN([data])

    Get the natural logarithm; LN(X) is the logarithm with the base e

    :param data: the input data
    :rtype: Indicator)");

    m.def("LOG", LOG_1);
    m.def("LOG", LOG_2);
    m.def("LOG", LOG_3, R"(LOG([data])

    The logarithm with the base 10

    :param data: the input data
    :rtype: Indicator)");

    m.def("HHVBARS", HHVBARS_1, py::arg("n") = 20);
    m.def("HHVBARS", HHVBARS_2, py::arg("n"));
    m.def("HHVBARS", HHVBARS_3, py::arg("data"), py::arg("n"));
    m.def("HHVBARS", HHVBARS_4, py::arg("data"), py::arg("n"));
    m.def("HHVBARS", HHVBARS_5, py::arg("data"), py::arg("n") = 20, R"(HHVBARS([data, n=20])

    The position of the previous high point, getting the number of the periods from the previous high point to the current.

    Usage: HHVBARS(X,N): getting the number of the periods from the highest value of X within the N periods to the current; N=0 means counting from the first valid value

    For example: HHVBARS(HIGH,0) gets the number of the periods from the historical new high to the current

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator)");

    m.def("LLVBARS", LLVBARS_1, py::arg("n") = 20);
    m.def("LLVBARS", LLVBARS_2, py::arg("n"));
    m.def("LLVBARS", LLVBARS_3, py::arg("data"), py::arg("n"));
    m.def("LLVBARS", LLVBARS_4, py::arg("data"), py::arg("n"));
    m.def("LLVBARS", LLVBARS_5, py::arg("data"), py::arg("n") = 20, R"(LLVBARS([data, n=20])

    The position of the previous low point, getting the number of the periods from the previous low point to the current.

    Usage: LLVBARS(X,N): getting the number of the periods from the lowest value of X within the N periods to the current; N=0 means counting from the first valid value

    For example: LLVBARS(HIGH,20) gets the number of the periods from the 20-day lowest point to the current

    :param data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator)");

    m.def("POW", POW_1, py::arg("n"));
    m.def("POW", POW_2, py::arg("n"));
    m.def("POW", POW_3, py::arg("data"), py::arg("n"));
    m.def("POW", POW_4, py::arg("data"), py::arg("n"));
    m.def("POW", POW_5, py::arg("data"), py::arg("n"));
    m.def("POW", POW_6, py::arg("data"), py::arg("n"), R"(POW(data, n)

    The power

    Usage: POW(A,B) returns A to the power of B

    For example: POW(CLOSE,3) gets the 3rd power of the close price

    :param data: the input data
    :param int|Indicator|IndParam n: the power
    :rtype: Indicator)");

    m.def("SIGNED_POWER", SIGNED_POWER_1, py::arg("n"));
    m.def("SIGNED_POWER", SIGNED_POWER_2, py::arg("n"));
    m.def("SIGNED_POWER", SIGNED_POWER_3, py::arg("data"), py::arg("n"));
    m.def("SIGNED_POWER", SIGNED_POWER_4, py::arg("data"), py::arg("n"));
    m.def("SIGNED_POWER", SIGNED_POWER_5, py::arg("data"), py::arg("n"));
    m.def("SIGNED_POWER", SIGNED_POWER_6, py::arg("data"), py::arg("n"), R"(SIGNED_POWER(data, n)

    The signed power

    Usage: SIGNED_POWER(A,B) returns A to the power of B, but keeping the original sign

    For example: SIGNED_POWER(CLOSE,3) gets the 3rd power of the close price, keeping the original sign

    :param data: the input data
    :param int|Indicator|IndParam n: the power
    :rtype: Indicator)");

    m.def("SQRT", SQRT_1);
    m.def("SQRT", SQRT_2);
    m.def("SQRT", SQRT_3, R"(SQRT([data])

    Take the square root

    Usage: SQRT(X) is the square root of X

    For example: SQRT(CLOSE) is the square root of the close price

    :param data: the input data
    :rtype: Indicator)");

    m.def("ROUND", ROUND_1, py::arg("ndigits") = 2);
    m.def("ROUND", ROUND_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUND", ROUND_3, py::arg("data"), py::arg("ndigits") = 2, R"(ROUND([data, ndigits=2])

    Round half up

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator)");

    m.def("ROUNDUP", ROUNDUP_1, py::arg("ndigits") = 2);
    m.def("ROUNDUP", ROUNDUP_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUNDUP", ROUNDUP_3, py::arg("data"), py::arg("ndigits") = 2,
          R"(ROUNDUP([data, ndigits=2])

    Round up, e.g. 10.1 becomes 11 after rounding

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator)");

    m.def("ROUNDDOWN", ROUNDDOWN_1, py::arg("ndigits") = 2);
    m.def("ROUNDDOWN", ROUNDDOWN_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUNDDOWN", ROUNDDOWN_3, py::arg("data"), py::arg("ndigits") = 2,
          R"(ROUNDDOWN([data, ndigits=2])

    Round down, e.g. 10.1 becomes 10 after rounding

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator)");

    m.def("FLOOR", FLOOR_1);
    m.def("FLOOR", FLOOR_2);
    m.def("FLOOR", FLOOR_3, R"(FLOOR([data])

    Round down (rounding in the direction of decreasing the value) to an integer

    Usage: FLOOR(A) returns the closest integer in the direction of decreasing the value of A

    For example: FLOOR(12.3) gets 12

    :param data: the input data
    :rtype: Indicator)");

    m.def("CEILING", CEILING_1);
    m.def("CEILING", CEILING_2);
    m.def("CEILING", CEILING_3, R"(CEILING([data])

    Round up (rounding in the direction of increasing the value) to an integer
   
    Usage: CEILING(A) returns the closest integer in the direction of increasing the value of A
   
    For example: CEILING(12.3) gets 13; CEILING(-3.5) gets -3
   
    :param data: the input data
    :rtype: Indicator)");

    m.def("INTPART", INTPART_1);
    m.def("INTPART", INTPART_2);
    m.def("INTPART", INTPART_3, R"(INTPART([data])

    Take the integer part (rounding by decreasing the absolute value, i.e. getting the integer part of the data)

    :param data: the input data
    :rtype: Indicator)");

    m.def("EXIST", EXIST_1, py::arg("n") = 20);
    m.def("EXIST", EXIST_2, py::arg("n"));
    m.def("EXIST", EXIST_3, py::arg("data"), py::arg("n"));
    m.def("EXIST", EXIST_4, py::arg("data"), py::arg("n"));
    m.def("EXIST", EXIST_5, py::arg("data"), py::arg("n") = 20, R"(EXIST([data, n=20])

    Exists; EXIST(X,N) means that the condition X exists within the N periods

    :param data: the input data
    :param int|Indicator|IndParam n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator)");

    m.def("EVERY", EVERY_1, py::arg("n") = 20);
    m.def("EVERY", EVERY_2, py::arg("n"));
    m.def("EVERY", EVERY_3, py::arg("data"), py::arg("n"));
    m.def("EVERY", EVERY_4, py::arg("data"), py::arg("n"));
    m.def("EVERY", EVERY_5, py::arg("data"), py::arg("n") = 20, R"(EVERY([data, n=20])

    Always exists

    Usage: EVERY (X,N) means that the condition X always exists within the N periods

    For example: EVERY(CLOSE>OPEN,10) means that all the previous 10 days are the bullish candles

    :param data: the input data
    :param int|Indicator|IndParam n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator)");

    m.def("LAST", LAST_1, py::arg("m") = 10, py::arg("n") = 5);
    m.def("LAST", LAST_2, py::arg("m"), py::arg("n"));
    m.def("LAST", LAST_3, py::arg("m"), py::arg("n") = 5);
    m.def("LAST", LAST_4, py::arg("m"), py::arg("n"));
    m.def("LAST", LAST_5, py::arg("data"), py::arg("m") = 10, py::arg("n") = 5);
    m.def("LAST", LAST_6, py::arg("data"), py::arg("m"), py::arg("n"));
    m.def("LAST", LAST_7, py::arg("data"), py::arg("m"), py::arg("n") = 5);
    m.def("LAST", LAST_8, py::arg("data"), py::arg("m"), py::arg("n"));
    m.def("LAST", LAST_9, py::arg("data"), py::arg("m"), py::arg("n"));
    m.def("LAST", LAST_10, py::arg("data"), py::arg("m"), py::arg("n") = 5);
    m.def("LAST", LAST_11, py::arg("data"), py::arg("m"), py::arg("n"), R"(LAST([data, m=10, n=5])

    Exists in the interval.

    Usage: LAST (X,M,N) means that the condition X exists from the previous M periods to the previous N periods.

    For example: LAST(CLOSE>OPEN,10,5) means the bullish candles from the previous 10 days to the previous 5 days.

    :param data: the input data
    :param int|Indicator|IndParam m: m periods
    :param int|Indicator|IndParam n: n periods
    :rtype: Indicator)");

    m.def("SIN", SIN_1);
    m.def("SIN", SIN_2);
    m.def("SIN", SIN_3, R"(SIN([data])

    The sine value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("ASIN", ASIN_1);
    m.def("ASIN", ASIN_2);
    m.def("ASIN", ASIN_3, R"(ASIN([data])

    The arcsine value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("COS", COS_1);
    m.def("COS", COS_2);
    m.def("COS", COS_3, R"(COS([data])

    The cosine value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("ACOS", ACOS_1);
    m.def("ACOS", ACOS_2);
    m.def("ACOS", ACOS_3, R"(ACOS([data])

    The arccosine value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("TAN", TAN_1);
    m.def("TAN", TAN_2);
    m.def("TAN", TAN_3, R"(TAN([data])

    The tangent value

    :param Indicator data: the input data
    :rtype: Indicato)");

    m.def("ATAN", ATAN_1);
    m.def("ATAN", ATAN_2);
    m.def("ATAN", ATAN_3, R"(ATAN([data])

    The arctangent value

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("REVERSE", REVERSE_1);
    m.def("REVERSE", REVERSE_2);
    m.def("REVERSE", REVERSE_3, R"(REVERSE([data])

    Get the opposite number; REVERSE(X) returns -X

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("MOD", MOD_1);
    m.def("MOD", MOD_2);
    m.def("MOD", MOD_3);
    m.def("MOD", MOD_4, R"(MOD(ind1, ind2)

    Get the modulus after taking the integer. This function is only for the TDX compatibility. In fact, the modulus of the indicators can be obtained directly with the % operator

    Usage: MOD(A,B) returns the modulus of A by B

    For example: MOD(26,10) returns 6

    :param Indicator ind1:
    :param Indicator ind2:
    :rtype: Indicator)");

    m.def("VAR", VAR_1, py::arg("n") = 10);
    m.def("VAR", VAR_2, py::arg("n"));
    m.def("VAR", VAR_3, py::arg("data"), py::arg("n"));
    m.def("VAR", VAR_4, py::arg("data"), py::arg("n"));
    m.def("VAR", VAR_5, py::arg("data"), py::arg("n") = 10, R"(VAR([data, n=10])

    The estimated sample variance; VAR(X,N) is the N-day estimated sample variance of X

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("VARP", VARP_1, py::arg("n") = 10);
    m.def("VARP", VARP_2, py::arg("n"));
    m.def("VARP", VARP_3, py::arg("data"), py::arg("n"));
    m.def("VARP", VARP_4, py::arg("data"), py::arg("n"));
    m.def("VARP", VARP_5, py::arg("data"), py::arg("n") = 10, R"(VARP([data, n=10])

    The population sample variance; VARP(X,N) is the N-day population sample variance of X

    :param Indicator data: the input data
    :param int n|Indicator|IndParam: the time window
    :rtype: Indicator)");

    m.def("UPNDAY", UPNDAY_1, py::arg("data"), py::arg("n") = 3);
    m.def("UPNDAY", UPNDAY_2, py::arg("data"), py::arg("n"));
    m.def("UPNDAY", UPNDAY_3, py::arg("data"), py::arg("n"), R"(UPNDAY(data[, n=3])

    The number of the consecutive rising periods; UPNDAY(CLOSE,M) means M consecutive rising periods

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("DOWNNDAY", DOWNNDAY_1, py::arg("data"), py::arg("n") = 3);
    m.def("DOWNNDAY", DOWNNDAY_2, py::arg("data"), py::arg("n"));
    m.def("DOWNNDAY", DOWNNDAY_3, py::arg("data"), py::arg("n"), R"(DOWNNDAY(data[, n=3])

    The number of the consecutive falling periods; DOWNNDAY(CLOSE,M) means M consecutive falling periods

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("NDAY", NDAY_1, py::arg("x"), py::arg("y"), py::arg("n") = 3);
    m.def("NDAY", NDAY_1, py::arg("x"), py::arg("y"), py::arg("n"));
    m.def("NDAY", NDAY_3, py::arg("x"), py::arg("y"), py::arg("n"), R"(NDAY(x, y[, n=3])

    Consecutively greater; NDAY(X,Y,N) means that the condition X>Y persists for N periods

    :param Indicator x:
    :param Indicator y:
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("CROSS", CROSS_1);
    m.def("CROSS", CROSS_2);
    m.def("CROSS", CROSS_3);
    m.def("CROSS", CROSS_4, R"(CROSS(x, y)

    The cross function

    :param x: the variable or the constant, the first line to judge the cross
    :param y: the variable or the constant, the second line to judge the cross
    :rtype: Indicator)");

    m.def("LONGCROSS", LONGCROSS_1, py::arg("a"), py::arg("b"), py::arg("n") = 3);
    m.def("LONGCROSS", LONGCROSS_2, py::arg("a"), py::arg("b"), py::arg("n"));
    m.def("LONGCROSS", LONGCROSS_3, py::arg("a"), py::arg("b"), py::arg("n") = 3);
    m.def("LONGCROSS", LONGCROSS_4, py::arg("a"), py::arg("b"), py::arg("n"));
    m.def("LONGCROSS", LONGCROSS_5, py::arg("a"), py::arg("b"), py::arg("n") = 3);
    m.def("LONGCROSS", LONGCROSS_6, py::arg("a"), py::arg("b"), py::arg("n"));
    m.def("LONGCROSS", LONGCROSS_7, py::arg("a"), py::arg("b"), py::arg("n") = 3);
    m.def("LONGCROSS", LONGCROSS_8, py::arg("a"), py::arg("b"), py::arg("n"),
          R"(LONGCROSS(a, b[, n=3])

    The two lines cross after maintaining for a certain period

    Usage: LONGCROSS(A,B,N) means that A is less than B within the N periods, and it returns 1 when A crosses B upward from below in this period, otherwise 0

    For example: LONGCROSS(MA(CLOSE,5),MA(CLOSE,10),5) means that the 5-day MA is maintained for 5 periods and then crosses the 10-day MA in a golden cross

    :param Indicator a:
    :param Indicator b:
    :param int|Indicator n:
    :rtype: Indicator)");

    m.def("FILTER", FILTER_1, py::arg("n") = 5);
    m.def("FILTER", FILTER_2, py::arg("n"));
    m.def("FILTER", FILTER_3, py::arg("data"), py::arg("n"));
    m.def("FILTER", FILTER_4, py::arg("data"), py::arg("n"));
    m.def("FILTER", FILTER_5, py::arg("data"), py::arg("n") = 5, R"(FILTER([data, n=5])

    The signal filter, filtering the signals that appear consecutively.

    Usage: FILTER(X,N): after X satisfies the condition, delete the data within the following N periods, setting them to 0.

    For example: FILTER(CLOSE>OPEN,5) searches for the bullish candles, and the bullish candles appearing again within 5 days are not recorded.

    :param Indicator data: the input data
    :param int|Indicaot|IndParam n: the filter period
    :rtype: Indicator)");

    m.def("BARSSINCE", BARSSINCE_1);
    m.def("BARSSINCE", BARSSINCE_2);
    m.def("BARSSINCE", BARSSINCE_3, R"(BARSSINCE([data])

    The number of the periods from the first time the condition was satisfied to the current.

    Usage: BARSSINCE(X): the number of the days from the first time X is not 0 to now.

    For example: BARSSINCE(HIGH>10) means the number of the periods from when the stock price exceeds 10 yuan to the current

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("BARSSINCEN", py::overload_cast<int>(BARSSINCEN), py::arg("n"));
    m.def("BARSSINCEN", py::overload_cast<const Indicator&, int>(BARSSINCEN), py::arg("cond"),
          py::arg("n"), R"(BARSSINCEN(cond, n)
    
    The number of the periods from the first time the condition is satisfied within the N periods to the current

    Usage: BARSSINCEN(X,N): the number of the periods from the first time X is not 0 within the N periods to now; N is a constant BARSSINCEN(X,N)
    For example: BARSSINCEN(HIGH>10,10) means the number of the periods from when the stock price exceeds 10 yuan within the 10 periods to the current

    :param Indicator cond: the condition
    :param int|Indicator n: the time window
    :rtype: Indicator)");

    m.def("BARSLAST", BARSLAST_1);
    m.def("BARSLAST", BARSLAST_2);
    m.def("BARSLAST", BARSLAST_3, R"(BARSLAST([data])

    The position of the last time the condition was satisfied, the number of the periods from the last time the condition was satisfied to the current.

    Usage: BARSLAST(X): the number of the days from the last time X is not 0 to now.

    For example: BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) means the number of the periods from the last limit-up board to the current

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("BARSLASTS", BARSLASTS_1);
    m.def("BARSLASTS", BARSLASTS_2, py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_3, py::arg("data"), py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_4, py::arg("data"), py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_5, py::arg("data"), py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_6, py::arg("val"), py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_7, py::arg("val"), py::arg("n"));
    m.def("BARSLASTS", BARSLASTS_8, py::arg("val"), py::arg("n"), R"(BARSLASTS([data|val,] n)

    The number of the periods from the Nth time the condition was satisfied to the current.

    Usage: BARSLASTS(X, N): the number of the days from the Nth time X is not 0 to now.

    For example: BARSLASTS(CLOSE/REF(CLOSE,1)>=1.1, 2) means the number of the periods from the 2nd limit-up board to the current.

    Note: when N=1, BARSLASTS(X, 1) is equivalent to BARSLAST(X).

    :param Indicator data: the input data (optional)
    :param float val: the input value (optional, choose one of data and it)
    :param int|Indicator|IndParam n: the Nth time the condition is satisfied, supporting the dynamic parameters
    :rtype: Indicator)");

    m.def("SUMBARS", SUMBARS_1, py::arg("a"));
    m.def("SUMBARS", SUMBARS_2, py::arg("a"));
    m.def("SUMBARS", SUMBARS_3, py::arg("data"), py::arg("a"));
    m.def("SUMBARS", SUMBARS_4, py::arg("data"), py::arg("a"));
    m.def("SUMBARS", SUMBARS_5, py::arg("data"), py::arg("a"), R"(SUMBARS([data,] a)

    Accumulate to the specified number of the periods, the number of the periods from accumulating forward to the specified value until now

    Usage: SUMBARS(X,A): accumulate X forward until it is greater than or equal to A, returning the number of the periods of this interval

    For example: SUMBARS(VOL,CAPITAL) gets the number of the periods from a complete turnover to now

    :param Indicator data: the input data
    :param float a|Indicator|IndParam: the specified accumulated sum
    :rtype: Indicator)");

    m.def("BARSCOUNT", BARSCOUNT_1);
    m.def("BARSCOUNT", BARSCOUNT_2, R"(BARSCOUNT([data])

    The number of the valid periods, getting the total number of the periods.

    Usage: BARSCOUNT(X), the number of the days from the first valid data to the current.

    For example: BARSCOUNT(CLOSE) gets the total number of the trading days since the listing for the daily data, and the number of the trading minutes of the day for the 1-minute line.

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("BACKSET", BACKSET_1, py::arg("n") = 2);
    m.def("BACKSET", BACKSET_2, py::arg("n"));
    m.def("BACKSET", BACKSET_3, py::arg("data"), py::arg("n"));
    m.def("BACKSET", BACKSET_4, py::arg("data"), py::arg("n"));
    m.def("BACKSET", BACKSET_5, py::arg("data"), py::arg("n") = 2, R"(BACKSET([data, n=2])

    The forward assignment, setting the data from the current position to several periods ago to 1.

    Usage: BACKSET(X,N); if X is not 0, set the values from the current position to N periods ago to 1.

    For example: BACKSET(CLOSE>OPEN,2); if it closes bullish, set the values of this period and the previous period to 1, otherwise 0

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: N periods
    :rtype: Indicator)");

    m.def("TIMELINE", TIMELINE_1);
    m.def("TIMELINE", TIMELINE_2, R"(TIMELINE([k])

    The time-line price data

    :param KData k: the context
    :rtype: Indicator)");

    m.def("TIMELINEVOL", TIMELINEVOL_1);
    m.def("TIMELINEVOL", TIMELINEVOL_2, R"(TIMELINEVOL([k])

    The time-line volume data

    :param KData k: the context
    :rtype: Indicator)");

    m.def("DMA", DMA, py::arg("x"), py::arg("a"), py::arg("fill_null") = true,
          R"(DMA(x, a[, fill_null=True])

    The dynamic moving average

    Usage: DMA(X,A), getting the dynamic moving average of X.

    The algorithm: if Y=DMA(X,A), then Y=A*X+(1-A)*Y', where Y' represents the Y value of the previous period.

    For example: DMA(CLOSE,VOL/CAPITAL) means getting the average price with the turnover rate as the smoothing factor

    :param Indicator x: the input data
    :param Indicator a: the dynamic coefficient
    :param bool fill_null: fill the missing data with the nan values during the date alignment.
    :rtype: Indicator)");

    m.def("AVEDEV", AVEDEV_1, py::arg("data"), py::arg("n") = 22);
    m.def("AVEDEV", AVEDEV_2, py::arg("data"), py::arg("n"));
    m.def("AVEDEV", AVEDEV_3, py::arg("data"), py::arg("n"), R"(AVEDEV(data[, n=22])

    The average absolute deviation, getting the N-day average absolute deviation of X

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("DEVSQ", DEVSQ_1, py::arg("n") = 10);
    m.def("DEVSQ", DEVSQ_2, py::arg("n"));
    m.def("DEVSQ", DEVSQ_3, py::arg("data"), py::arg("n"));
    m.def("DEVSQ", DEVSQ_4, py::arg("data"), py::arg("n"));
    m.def("DEVSQ", DEVSQ_5, py::arg("data"), py::arg("n") = 10, R"(DEVSQ([data, n=10])

    The sum of the squared deviations of the data, getting the N-day sum of the squared deviations of X

    :param Indicator data: the input data
    :param int|Indicator n: the time window
    :rtype: Indicator)");

    m.def("ROC", ROC_1, py::arg("n") = 10);
    m.def("ROC", ROC_2, py::arg("n"));
    m.def("ROC", ROC_3, py::arg("data"), py::arg("n"));
    m.def("ROC", ROC_4, py::arg("data"), py::arg("n"));
    m.def("ROC", ROC_5, py::arg("data"), py::arg("n") = 10, R"(ROC([data, n=10])

    The rate of change indicator: ((price / prevPrice)-1)*100

    :param data: the input data
    :param int n: the time window
    :rtype: Indicator)");

    m.def("ROCP", ROCP_1, py::arg("n") = 10);
    m.def("ROCP", ROCP_2, py::arg("n"));
    m.def("ROCP", ROCP_3, py::arg("data"), py::arg("n"));
    m.def("ROCP", ROCP_4, py::arg("data"), py::arg("n"));
    m.def("ROCP", ROCP_5, py::arg("data"), py::arg("n") = 10, R"(ROCP([data, n=10])

    The rate of change indicator: (price - prevPrice) / prevPrice

    :param data: the input data
    :param int n: the time window
    :rtype: Indicator)");

    m.def("STKTYPE", STKTYPE1);
    m.def("STKTYPE", STKTYPE2, py::arg("k"), R"(STKTYPE(k)

    Get the stock type indicator

    Return the type value (the StockType enumeration value) of the current stock

    :param KData k: the K-line data context
    :rtype: Indicator)");

    m.def("ROCR", ROCR_1, py::arg("n") = 10);
    m.def("ROCR", ROCR_2, py::arg("n"));
    m.def("ROCR", ROCR_3, py::arg("data"), py::arg("n"));
    m.def("ROCR", ROCR_4, py::arg("data"), py::arg("n"));
    m.def("ROCR", ROCR_5, py::arg("data"), py::arg("n") = 10, R"(ROCR([data, n=10])

    The rate of change indicator: (price / prevPrice)

    :param data: the input data
    :param int n|Indicator|IndParam: the time window
    :rtype: Indicator)");

    m.def("ROCR100", ROCR100_1, py::arg("n") = 10);
    m.def("ROCR100", ROCR100_2, py::arg("n"));
    m.def("ROCR100", ROCR100_3, py::arg("data"), py::arg("n"));
    m.def("ROCR100", ROCR100_4, py::arg("data"), py::arg("n"));
    m.def("ROCR100", ROCR100_5, py::arg("data"), py::arg("n") = 10, R"(ROCR100([data, n=10])

    The rate of change indicator: (price / prevPrice) * 100

    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("AD", AD_1);
    m.def("AD", AD_2, R"(AD(kdata)

   The accumulation/distribution line

   :param KData kdata: the K-line data
   :rtype: Indicator)");

    m.def("ADJ_FACTOR", py::overload_cast<const KData&>(&ADJ_FACTOR));
    m.def("ADJ_FACTOR", py::overload_cast<>(&ADJ_FACTOR), R"(ADJ_FACTOR()

    Calculate the recovery factor indicator

    Calculate the backward recovery factor sequence based on the stock's dividend data (the stock sends, the allotments, the conversions, the cash dividends, etc.).
    The recovery factor indicates how many shares are held now if 1 share was held at the listing, after all the stock sends, the allotments and the conversions.
    It is calculated in a cumulative multiplication way, ensuring the consistency of the recovery processing of the price, the volume and the amount.

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovery factor indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovery factor of a stock
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_factor = ADJ_FACTOR()
        adj_factor.set_context(kdata)

    .. seealso::

        - `ADJ_OPEN` - the recovered open price
        - `ADJ_HIGH` - the recovered high price
        - `ADJ_LOW` - the recovered low price
        - `ADJ_CLOSE` - the recovered close price
        - `ADJ_VOL` - the recovered volume
    )");

    m.def("ADJ_OPEN", py::overload_cast<const KData&>(&ADJ_OPEN));
    m.def("ADJ_OPEN", py::overload_cast<>(&ADJ_OPEN), R"(ADJ_OPEN()

    Calculate the recovered open price indicator

    Recover the open price backward by the recovery factor, obtaining the recovered open price sequence.
    The calculation formula: ADJ_OPEN = ADJ_FACTOR * OPEN

    **The design purpose**:
    The ADJ_ series indicators are mainly designed to work with the factor management system to calculate the backward equal-ratio recovery factors quickly.
    In the factor management scenarios, the recovery calculation can be performed efficiently by updating and storing the factor values incrementally every day.

    **Important restrictions**:
    - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
    - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
    - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
    - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovered open price indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovered open price of a stock (the factor management scenario)
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_open = ADJ_OPEN()
        adj_open.set_context(kdata)

    .. seealso::

        - `ADJ_FACTOR` - the recovery factor
        - `ADJ_HIGH` - the recovered high price
        - `ADJ_LOW` - the recovered low price
        - `ADJ_CLOSE` - the recovered close price
        - `ADJ_VOL` - the recovered volume
        - `RECOVER_EQUAL_FORWARD` - the equal-ratio forward recovery (recommended for the non-factor-management scenarios)
    )");

    m.def("ADJ_HIGH", py::overload_cast<const KData&>(&ADJ_HIGH));
    m.def("ADJ_HIGH", py::overload_cast<>(&ADJ_HIGH), R"(ADJ_HIGH()

    Calculate the recovered high price indicator

    Recover the high price backward by the recovery factor, obtaining the recovered high price sequence.
    The calculation formula: ADJ_HIGH = ADJ_FACTOR * HIGH

    **The design purpose**:
    The ADJ_ series indicators are mainly designed to work with the factor management system to calculate the backward equal-ratio recovery factors quickly.
    In the factor management scenarios, the recovery calculation can be performed efficiently by updating and storing the factor values incrementally every day.

    **Important restrictions**:
    - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
    - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
    - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
    - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovered high price indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovered high price of a stock (the factor management scenario)
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_high = ADJ_HIGH()
        adj_high.set_context(kdata)

    .. seealso::

        - `ADJ_FACTOR` - the recovery factor
        - `ADJ_OPEN` - the recovered open price
        - `ADJ_LOW` - the recovered low price
        - `ADJ_CLOSE` - the recovered close price
        - `ADJ_VOL` - the recovered volume
        - `RECOVER_EQUAL_FORWARD` - the equal-ratio forward recovery (recommended for the non-factor-management scenarios)
    )");

    m.def("ADJ_LOW", py::overload_cast<const KData&>(&ADJ_LOW));
    m.def("ADJ_LOW", py::overload_cast<>(&ADJ_LOW), R"(ADJ_LOW()

    Calculate the recovered low price indicator

    Recover the low price backward by the recovery factor, obtaining the recovered low price sequence.
    The calculation formula: ADJ_LOW = ADJ_FACTOR * LOW

    **The design purpose**:
    The ADJ_ series indicators are mainly designed to work with the factor management system to calculate the backward equal-ratio recovery factors quickly.
    In the factor management scenarios, the recovery calculation can be performed efficiently by updating and storing the factor values incrementally every day.

    **Important restrictions**:
    - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
    - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
    - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
    - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovered low price indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovered low price of a stock (the factor management scenario)
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_low = ADJ_LOW()
        adj_low.set_context(kdata)

    .. seealso::

        - `ADJ_FACTOR` - the recovery factor
        - `ADJ_OPEN` - the recovered open price
        - `ADJ_HIGH` - the recovered high price
        - `ADJ_CLOSE` - the recovered close price
        - `ADJ_VOL` - the recovered volume
        - `RECOVER_EQUAL_FORWARD` - the equal-ratio forward recovery (recommended for the non-factor-management scenarios)
    )");

    m.def("ADJ_CLOSE", py::overload_cast<const KData&>(&ADJ_CLOSE));
    m.def("ADJ_CLOSE", py::overload_cast<>(&ADJ_CLOSE), R"(ADJ_CLOSE()

    Calculate the recovered close price indicator

    Recover the close price backward by the recovery factor, obtaining the recovered close price sequence.
    The calculation formula: ADJ_CLOSE = ADJ_FACTOR * CLOSE

    **The design purpose**:
    The ADJ_ series indicators are mainly designed to work with the factor management system to calculate the backward equal-ratio recovery factors quickly.
    In the factor management scenarios, the recovery calculation can be performed efficiently by updating and storing the factor values incrementally every day.

    **Important restrictions**:
    - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
    - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
    - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
    - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovered close price indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovered close price of a stock (the factor management scenario)
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_close = ADJ_CLOSE()
        adj_close.set_context(kdata)

    .. seealso::

        - `ADJ_FACTOR` - the recovery factor
        - `ADJ_OPEN` - the recovered open price
        - `ADJ_HIGH` - the recovered high price
        - `ADJ_LOW` - the recovered low price
        - `ADJ_VOL` - the recovered volume
        - `RECOVER_EQUAL_FORWARD` - the equal-ratio forward recovery (recommended for the non-factor-management scenarios)
    )");

    m.def("ADJ_VOL", py::overload_cast<const KData&>(&ADJ_VOL));
    m.def("ADJ_VOL", py::overload_cast<>(&ADJ_VOL), R"(ADJ_VOL()

    Calculate the recovered volume indicator

    Recover the volume backward by the recovery factor, obtaining the recovered volume sequence.
    The calculation formula: ADJ_VOL = VOL / ADJ_FACTOR

    **The design purpose**:
    The ADJ_ series indicators are mainly designed to work with the factor management system to calculate the backward equal-ratio recovery factors quickly.
    In the factor management scenarios, the recovery calculation can be performed efficiently by updating and storing the factor values incrementally every day.

    **Note**: the volume recovery uses division, which is opposite to the price recovery using multiplication.
    This is because when the share capital increases, the volume corresponding to each share should be reduced accordingly.

    **Important restrictions**:
    - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
    - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
    - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
    - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query

    This indicator needs to set the KData context to work properly, set through the setContext() method.

    :return: the recovered volume indicator object
    :rtype: Indicator

    .. code-block:: python

        # Get the recovered volume of a stock (the factor management scenario)
        stock = sm['sh000001']
        kdata = stock.get_kdata(Query(-100))
        adj_vol = ADJ_VOL()
        adj_vol.set_context(kdata)

    .. seealso::

        - `ADJ_FACTOR` - the recovery factor
        - `ADJ_OPEN` - the recovered open price
        - `ADJ_HIGH` - the recovered high price
        - `ADJ_LOW` - the recovered low price
        - `ADJ_CLOSE` - the recovered close price
        - `RECOVER_EQUAL_FORWARD` - the equal-ratio forward recovery (recommended for the non-factor-management scenarios)
    )");

    m.def("COST", COST_1, py::arg("x") = 10.0);
    m.def("COST", COST_2, py::arg("k"), py::arg("x") = 10.0, R"(COST(k[, x=10.0])

    The cost distribution. This function is only valid for the daily analysis period, and invalid for the indexes, the ETFs, etc. that have no circulating share capital dividend data.
    Usage: COST(k, X) means what the price of the X% profitable positions is
    For example: COST(k, 10) means what the price of the 10% profitable positions is, i.e. 10% of the positions are below this price, and the other 90% are above this price, being the trapped positions

    :param KData k: the associated K-line data
    :param float x: the x% profitable price, 0~100
    :rtype: Indicator)");

    m.def("ALIGN", ALIGN_1, py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_2, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_3, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_4, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true,
          R"(ALIGN(data, ref):

    Align by the specified reference dates

    :param Indicator data: the input data
    :param DatetimeList|Indicator|KData ref: the specified DatetimeList, Indicator or KData used as the date reference
    :param bool fill_null: the missing data is filled with nan; otherwise, use the data less than and closest to the corresponding date
    :retype: Indicator)");

    m.def("DROPNA", DROPNA_1);
    m.def("DROPNA", DROPNA_2, R"(DROPNA([data])

    Remove the nan values

    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("ADVANCE", ADVANCE, py::arg("query") = KQueryByIndex(-100), py::arg("market") = "SH",
          py::arg("stk_type") = STOCKTYPE_A, py::arg("ignore_context") = false,
          py::arg("fill_null") = true,
          R"(ADVANCE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    The number of the rising stocks. When a specified context exists and ignore_context is false, the query, market, stk_type parameters will be ignored.

    :param Query query: the query condition
    :param str market: the market it belongs to; when it equals "", get all the markets
    :param int stk_type: the security type; when it is greater than constant.STOCKTYPE_TMP, get all the types of the securities
    :param bool ignore_context: whether to ignore the context. When ignoring, the query, market, stk_type parameters are used forcibly.
    :para. bool fill_null: the missing data is filled with nan; otherwise, use the data less than and closest to the corresponding date
    :rtype: Indicator)");

    m.def("DECLINE", DECLINE, py::arg("query") = KQueryByIndex(-100), py::arg("market") = "SH",
          py::arg("stk_type") = STOCKTYPE_A, py::arg("ignore_context") = false,
          py::arg("fill_null") = true,
          R"(DECLINE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    The number of the falling stocks. When a specified context exists and ignore_context is false, the query, market, stk_type parameters will be ignored.

    :param Query query: the query condition
    :param str market: the market it belongs to; when it equals "", get all the markets
    :param int stk_type: the security type; when it is greater than constant.STOCKTYPE_TMP, get all the types of the securities
    :param bool ignore_context: whether to ignore the context. When ignoring, the query, market, stk_type parameters are used forcibly.
    :param bool fill_null: the missing data is filled with nan; otherwise, use the data less than and closest to the corresponding date
    :rtype: Indicator)");

    m.def("SLICE", SLICE_1, py::arg("data"), py::arg("start"), py::arg("end"));
    m.def("SLICE", SLICE_2, py::arg("start"), py::arg("end"), py::arg("result_index") = -1);
    m.def("SLICE", SLICE_3, py::arg("data"), py::arg("start"), py::arg("end"),
          py::arg("result_index") = 0,
          R"(SLICE(data, start, end, result_index=-1)

    Get the data of the specified range [start, end) in an indicator, generating a new indicator

    :param Indicator|PriceList data: the input data
    :param int start: the start position
    :param int end: the end position (exclusive)
    :param int result_index: the result set in the original input data; when it is less than 0, it means all the result sets)");

    m.def("RSI", RSI_1, py::arg("n") = 14);
    m.def("RSI", RSI_2, py::arg("data"), py::arg("n") = 14, R"(RSI([data, n=14])

    The Relative Strength Index

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator)");

    m.def("SLOPE", SLOPE1, py::arg("n") = 22);
    m.def("SLOPE", SLOPE2, py::arg("n"));
    m.def("SLOPE", SLOPE3, py::arg("data"), py::arg("n") = 22);
    m.def("SLOPE", SLOPE4, py::arg("data"), py::arg("n"));
    m.def("SLOPE", SLOPE5, py::arg("data"), py::arg("n"), R"(SLOPE([data, n=22])

    Calculate the linear regression slope, the goodness of fit R² and the relative maximum residual; N supports the variables

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator

    **The result sets**:

    * result(0): the slope
    * result(1): the goodness of fit R²
    * result(2): the relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ

    **The explanation of the relative maximum residual**:

    * The numerator: the maximum absolute residual within the window (guaranteeing that no point is far away from the regression line)
    * The denominator: the average of y within the window (eliminating the price dimension, with the percentage meaning)
    * The smaller the indicator = all the K-lines of the whole segment are close to the regression line

    **The examples of the judgment thresholds**:

    * RelMaxRes < 0.03: the farthest K-line deviates from the average price by less than 3%, and all the points are close to the regression line)");

    m.def("MDD", py::overload_cast<int>(&MDD), py::arg("n") = 0);
    m.def("MDD", py::overload_cast<const Indicator&, int>(&MDD), py::arg("data"), py::arg("n") = 0,
          R"(MDD([data, n=0])
    
    The maximum drawdown percentage (when n=0, there is no time window limit), which is a positive value by the industry convention
    
    :param Indicator data: the input data
    :param int n: the time window
    :rtype: Indicator)");

    m.def("MDD_CURRENT", py::overload_cast<>(&MDD_CURRENT));
    m.def("MDD_CURRENT", py::overload_cast<const Indicator&>(&MDD_CURRENT), py::arg("data"),
          R"(MDD_CURRENT([data])
    
    The drawdown percentage from the current point to the historical highest point, which is a positive value by the industry convention
    
    :param Indicator data: the input data
    :rtype: Indicator)");

    m.def("MRR", py::overload_cast<int>(&MRR), py::arg("n") = 0);
    m.def("MRR", py::overload_cast<const Indicator&, int>(&MRR), py::arg("data"), py::arg("n"),
          R"(MRR([data])
    
    The maximum profit percentage (calculated in the opposite direction corresponding to MDD)

    :param Indicator data: the input data
    :param int n: the time window
    :rtype: Indicator)");

    m.def("ZHBOND10", ZHBOND10_1, py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_2, py::arg("data"), py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_3, py::arg("data"), py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_4, py::arg("data"), py::arg("default_val") = 0.4,
          R"(ZHBOND10([data, defaut_val])

    Get the 10-year Chinese treasury bond yield

    :param DatetimeList|KDate|Indicator data: the input date reference, preferring the dates in the context
    :param float default_val: if the input date is earlier than the earliest record of the existing treasury bond data, this default value is used)");

    m.def("SPEARMAN", SPEARMAN_1, py::arg("ref_ind"), py::arg("n") = 0,
          py::arg("fill_null") = true);
    m.def("SPEARMAN", SPEARMAN_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 0,
          py::arg("fill_null") = true,
          R"(SPEARMAN(ind, ref_ind[, n=0, fill_null=True])

    The Spearman correlation coefficient. Equivalent to SPEARMAN(ref_ind, n)(ind).

    :param Indicator ind: the input parameter 1
    :param Indicator ref_ind: the input parameter 2
    :param int n: the rolling window (greater than 2 or equal to 0); when it is equal to 0, it means that n actually uses the length of the ind
    :param bool fill_null: the missing data is filled with nan; otherwise, use the data less than and closest to the corresponding date)");

    m.def("SKEW", SKEW_1, py::arg("n") = 10);
    m.def("SKEW", SKEW_2, py::arg("n"));
    m.def("SKEW", SKEW_4, py::arg("data"), py::arg("n"));
    m.def("SKEW", SKEW_5, py::arg("data"), py::arg("n"));
    m.def("SKEW", SKEW_3, py::arg("data"), py::arg("n") = 10, R"(SKEW([data, n=10])

    Calculate the unadjusted population skewness within the N periods

    :param Indicator data: the input data
    :param int n: the N-day time window (greater than or equal to 3 or equal to 0); when it is equal to 0, use the actual length of the input data
    :rtype: Indicator)");

    m.def("KURT", KURT_1, py::arg("n") = 10);
    m.def("KURT", KURT_2, py::arg("n"));
    m.def("KURT", KURT_4, py::arg("data"), py::arg("n"));
    m.def("KURT", KURT_5, py::arg("data"), py::arg("n"));
    m.def("KURT", KURT_3, py::arg("data"), py::arg("n") = 10, R"(KURT([data, n=10])

    Calculate the excess kurtosis within the N periods (the unadjusted population kurtosis - 3)

    :param Indicator data: the input data
    :param int n: the N-day time window (greater than or equal to 4 or equal to 0); when it is equal to 0, use the actual length of the input data
    :rtype: Indicator)");

    // IR(const Indicator& p, const Indicator& b, int n = 100)
    m.def("IR", IR, py::arg("p"), py::arg("b"), py::arg("n") = 100, R"(IR(p, b[, n])

    The Information Ratio (IR)

    The formula: (P-B) / TE
    P: the portfolio return
    B: the benchmark return
    TE: the standard deviation between p and b of each day within the investment period
    In actual use, P is generally the asset curve of the TM, and B is the close price of the CSI 300, e.g.:
    ref_k = sm["sh000300"].get_kdata(query)
    funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
    ir = IR(PRICELIST(funds), ref_k.close, 0)

    :param Indicator p:
    :param Indicator b:
    :param int n: the time window; if you only want to use the last value, you can use 0, or specify len(p), len(b)
    )");

    m.def(
      "IC",
      [](const Indicator& ind, const py::object& stks, int n, bool spearman, bool strict) {
          StockList c_stks = get_stock_list_from_python(stks);
          return IC(ind, c_stks, n, spearman, strict);
      },
      py::arg("ind"), py::arg("stks"), py::arg("n") = 1, py::arg("spearman") = true,
      py::arg("strict") = false,
      R"(IC(ind, stks[, n=1, spearman=True, strict=False]) -> Indicator

    Calculate the IC of the specified factor relative to the reference securities (actually the RankIC)

    The IC originally needs "the factor value at time t → the return at time t+1"; here it is changed to calculate "the factor value at time t → the return of the previous N days at time t" (e.g. the return of the past 5 days).
    (Otherwise the current values would all be missing NA), which is equivalent to shifting the original predicted IC right by n positions.

    If the strict "the factor value at time t → the return at time t+1" calculation is needed, please set strict=True (note that in this mode, the last n positions are NA)
    
    :param Indicator ind: the input factor
    :param sequence(stock)|Block stks the securities portfolio
    :param int n: the time window
    :param bool spearman: use the spearman correlation coefficient, otherwise pearson
    :param bool strict: the strict mode)");

    m.def(
      "ICIR",
      [](const Indicator& ind, const py::object& stks, int n, int rolling_n, bool spearman,
         bool strict) {
          StockList c_stks = get_stock_list_from_python(stks);
          return ICIR(ind, c_stks, n, rolling_n, spearman, strict);
      },
      py::arg("ind"), py::arg("stks"), py::arg("n") = 1, py::arg("rolling_n") = 120,
      py::arg("spearman") = true, py::arg("strict") = false,
      R"(ICIR(ind, stks[, n=1, rolling_n=120, spearman=True, strict=False])

    Calculate the IC factor IR = the multi-period average of the IC / the standard deviation of the IC

    :param Indicator ind: the input factor
    :param sequence(stock)|Block stks the securities portfolio
    :param int n: the n-day return corresponding when calculating the IC
    :param int rolling_n: the rolling period
    :param bool spearman: use the spearman correlation coefficient, otherwise pearson
    :param bool strict: whether the strict IC mode)");

    m.def("ZSCORE", ZSCORE_1, py::arg("out_extreme") = false, py::arg("nsigma") = 3.0,
          py::arg("recursive") = false);
    m.def("ZSCORE", ZSCORE_2, py::arg("data"), py::arg("out_extreme") = false,
          py::arg("nsigma") = 3.0, py::arg("recursive") = false,
          R"(ZSCORE(data[, out_extreme, nsigma, recursive])

    Standardize (normalize) the data, with the optional outlier processing

    Note: it is not a window rolling; if the standardization with the window rolling is needed, just use (x - MA(x, n)) / STDEV(x, n).
    
    :param Indicator data: the data from which the outliers are to be removed
    :param bool out_extreme: indicate removing the extreme values, defaulting to False
    :param float nsigma: the multiple of sigma used when removing the extreme values, defaulting to 3.0
    :param bool recursive: whether to remove the extreme values recursively, defaulting to False
    :rtype: Indicator)");

    m.def("QUANTILE_TRUNC", py::overload_cast<int, double, double>(QUANTILE_TRUNC),
          py::arg("n") = 60, py::arg("quantial_min") = 0.01, py::arg("quantial_max") = 0.99);
    m.def("QUANTILE_TRUNC",
          py::overload_cast<const Indicator&, int, double, double>(QUANTILE_TRUNC), py::arg("data"),
          py::arg("n") = 60, py::arg("quantial_min") = 0.01, py::arg("quantial_max") = 0.99,
          R"(QUANTILE_TRUNC(data[, n=60, quantial_min=0.01, quantial_max=0.99])

    Perform the quantile truncation on the data. Not a window rolling.

    :param Indicator data: the data from which the outliers are to be removed
    :param int n: the time window
    :param float quantial_min: the lower limit of the percentile used when removing the extreme values, defaulting to 0.01
    :param float quantial_max: the upper limit of the percentile used when removing the extreme values, defaulting to 0.99
    :rtype: Indicator)");

    m.def("TURNOVER", py::overload_cast<int>(TURNOVER), py::arg("n") = 1);
    m.def("TURNOVER", py::overload_cast<const KData&, int>(TURNOVER), py::arg("kdata"),
          py::arg("n") = 1, R"(TURNOVER(data[,n=1])
    The turnover rate = the stock volume / the number of the circulating shares × 100%

    :param int n: the time window)");

    m.def("TS_RANK", py::overload_cast<int>(TS_RANK), py::arg("n") = 20);
    m.def("TS_RANK", py::overload_cast<const Indicator&, int>(TS_RANK), py::arg("data"),
          py::arg("n") = 20, R"(TS_RANK([data, n=20])

    The time series ranking, calculating the ranking ratio of the current value within the past N periods (Alpha101)

    Usage: TS_RANK(X,N) means the ranking of X within the past N periods (from 1 to N) divided by N
    For example: TS_RANK(CLOSE,20) means the ranking ratio of the close price within the past 20 periods

    The definition in Alpha101:
    TS_RANK(x, n) = (rank of x in the last n observations) / n
    where the rank is the ascending ranking, i.e. the smaller values are ranked higher, and the output is the percentile value of 0~1

    :param Indicator data: the data to calculate
    :param int n: the number of the periods, defaulting to 20
    :rtype: Indicator)");

    m.def("RESULT", py::overload_cast<int>(RESULT));
    m.def("RESULT", py::overload_cast<const Indicator&, int>(RESULT), py::arg("data"),
          py::arg("result_ix"), R"(RESULT(data, result_ix)
          
    Return the specified result set in the specified indicator in the way of the formula indicator

    :param Indicator data: the specified indicator
    :param int result_ix: the specified result set)");

    m.def("FINANCE", py::overload_cast<int>(FINANCE), py::arg("ix"));
    m.def("FINANCE", py::overload_cast<const string&>(FINANCE), py::arg("name"));
    m.def("FINANCE", py::overload_cast<const KData&, int>(FINANCE), py::arg("kdata"),
          py::arg("ix"));
    m.def("FINANCE", py::overload_cast<const KData&, const string&>(FINANCE), py::arg("kdata"),
          py::arg("name"),
          R"(FINANCE([kdata, ix, name])

    Get the historical finance information. (The corresponding historical finance field information can be queried through StockManager.get_history_finance_all_fields)

    When ix and name are used, choose one of them. That is, either use ix or use name to get.

    :param KData kdata: the K-line data
    :param int ix: the historical finance information field index
    :param int name: the historical finance information field name)");

    m.def("BLOCKSETNUM", py::overload_cast<const Block&>(BLOCKSETNUM), py::arg("block"));
    m.def("BLOCKSETNUM", py::overload_cast<const Block&, const KQuery&>(BLOCKSETNUM),
          py::arg("block"), py::arg("query"), R"(BLOCKSETNUM(block, query)
    
    The cross-sectional statistics (returning the number of the stocks in the block)

    :param Block block: the block to count
    :param Query query: the statistics range)");

    m.def(
      "BLOCKSETNUM",
      [](const py::object& stks) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return BLOCKSETNUM(blk);
      },
      py::arg("stks"));
    m.def(
      "BLOCKSETNUM",
      [](const py::object& stks, const KQuery& query) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return BLOCKSETNUM(blk, query);
      },
      py::arg("stks"), py::arg("query"), R"(BLOCKSETNUM(block, query)
    
    The cross-sectional statistics (returning the number of the stocks in the block)

    :param Sequence stks: stock list
    :param Query query: the statistics range)");

    m.def("INSUM", py::overload_cast<const Block&, int, bool>(INSUM), py::arg("block"),
          py::arg("mode"), py::arg("fill_null") = true);
    m.def("INSUM", py::overload_cast<const Block&, const Indicator&, int, bool>(INSUM),
          py::arg("block"), py::arg("ind"), py::arg("mode"), py::arg("fill_null") = true);
    m.def("INSUM",
          py::overload_cast<const Block&, const KQuery&, const Indicator&, int, bool>(INSUM),
          py::arg("block"), py::arg("query"), py::arg("ind"), py::arg("mode"),
          py::arg("fill_null") = true);
    m.def(
      "INSUM",
      [](const py::sequence stks, int mode, bool fill_null) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return INSUM(blk, mode, fill_null);
      },
      py::arg("stks"), py::arg("mode"), py::arg("fill_null") = true);
    m.def(
      "INSUM",
      [](const py::sequence stks, const Indicator& ind, int mode, bool fill_null) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return INSUM(blk, ind, mode);
      },
      py::arg("stks"), py::arg("ind"), py::arg("mode"), py::arg("fill_null") = true);
    m.def(
      "INSUM",
      [](const py::sequence stks, const KQuery& query, const Indicator& ind, int mode,
         bool fill_null) {
          Block blk;
          blk.add(get_stock_list_from_python(stks));
          return INSUM(blk, query, ind, mode);
      },
      py::arg("stks"), py::arg("query"), py::arg("ind"), py::arg("mode"),
      py::arg("fill_null") = true,
      R"(INSUM(stks, query, ind, mode[, fill_null=True])

    Return the calculated values of the corresponding outputs of this indicator for each constituent in the block, aggregated by the calculation type. The calculation types: 0-accumulation, 1-average, 2-maximum, 3-minimum, 4-descending ranking, 5-ascending ranking.

    Note: when the INSUM uses the modes 4/5, it is equivalent to the RANK function, but it is not suitable for use in the MF; when used in the MF, the calculation amount is at the N x N level, calculating slowly. If you want to use it in the MF, it is recommended to use the RANK indicator directly.

    :param Sequence stks: stock list
    :param Query query: the specified range
    :param Indicator ind: the specified indicator
    :param int mode: the calculation type: 0-accumulation, 1-average, 2-maximum, 3-minimum, 4-descending ranking (the highest indicator value is ranked 1), 5-ascending ranking (the greater the indicator value, the greater the ranking value)
    :param bool fill_null: fill the missing data with the nan values during the date alignment.
    :rtype: Indicator)");

    m.def("ISLASTBAR", py::overload_cast<>(ISLASTBAR));
    m.def("ISLASTBAR", py::overload_cast<const KData&>(ISLASTBAR), py::arg("data"));
    m.def("ISLASTBAR", py::overload_cast<const Indicator&>(ISLASTBAR), py::arg("data"),
          R"(ISLASTBAR(ind)

    Judge whether the current data is the last data; if it is the last data, return 1, otherwise 0.

    :param Indicator|KData data: the specified indicator
    :rtype: Indicator)");

    m.def("ISNA", py::overload_cast<bool>(ISNA), py::arg("ignore_discard") = false);
    m.def("ISNA", py::overload_cast<const Indicator&, bool>(ISNA), py::arg("ind"),
          py::arg("ignore_discard") = false, R"(ISNA(ind[, ignore_discard=False])

    Judge whether the indicator is a nan value; if it is a nan value, return 1, otherwise 0.

    :param Indicator ind: the specified indicator
    :param bool ignore_discard: ignore the discarded data of the indicator)");

    m.def("ISINF", py::overload_cast<>(ISINF));
    m.def("ISINF", py::overload_cast<const Indicator&>(ISINF), py::arg("ind"),
          R"(ISINF(ind)

    Judge whether the indicator is a positive infinity (+inf) value; if it is a +inf value, return 1, otherwise 0. To judge the negative infinity, use ISINFA.

    :param Indicator ind: the specified indicator
    :rtype: Indicator)");

    m.def("ISINFA", py::overload_cast<>(ISINFA));
    m.def("ISINFA", py::overload_cast<const Indicator&>(ISINFA), py::arg("ind"),
          R"(ISINFA(ind)

    Judge whether the indicator is a negative infinity (-inf) value; if it is a -inf value, return 1, otherwise 0. To judge the positive infinity, use ISINF.

    :param Indicator ind: the specified indicator
    :rtype: Indicator)");

    double nan = Null<double>();
    m.def("REPLACE", py::overload_cast<double, double, bool>(REPLACE), py::arg("old_value") = nan,
          py::arg("new_value") = 0.0, py::arg("ignore_discard") = false);
    m.def("REPLACE", py::overload_cast<const Indicator&, double, double, bool>(REPLACE),
          py::arg("ind"), py::arg("old_value") = nan, py::arg("new_value") = 0.0,
          py::arg("ignore_discard") = false,
          R"(REPLACE(ind, [old_value=constant.nan, new_value=0.0, ignore_discard=False]
          
    Replace the specified values in the indicator, defaulting to replacing the nan values with 0.0.

    :param Indicator ind: the specified indicator
    :param double old_value: the specified value
    :param double new_value: the replacement value
    :param bool ignore_discard: ignore the discarded data of the indicator
    :rtype: Indicator)");

    m.def("INDEXO", py::overload_cast<bool>(INDEXO), py::arg("fill_null") = true);
    m.def("INDEXO", py::overload_cast<const KData&, bool>(INDEXO), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXO([kdata])
    
    Return the corresponding market open prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXH", py::overload_cast<bool>(INDEXH), py::arg("fill_null") = true);
    m.def("INDEXH", py::overload_cast<const KData&, bool>(INDEXH), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXH([kdata])
    
    Return the corresponding market high prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXL", py::overload_cast<bool>(INDEXL), py::arg("fill_null") = true);
    m.def("INDEXL", py::overload_cast<const KData&, bool>(INDEXL), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXL([kdata])
    
    Return the corresponding market low prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXC", py::overload_cast<bool>(INDEXC), py::arg("fill_null") = true);
    m.def("INDEXC", py::overload_cast<const KData&, bool>(INDEXC), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXC([kdata])
    
    Return the corresponding market close prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXV", py::overload_cast<bool>(INDEXV), py::arg("fill_null") = true);
    m.def("INDEXV", py::overload_cast<const KData&, bool>(INDEXV), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXV([kdata])
    
    Return the corresponding market volumes, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXA", py::overload_cast<bool>(INDEXA), py::arg("fill_null") = true);
    m.def("INDEXA", py::overload_cast<const KData&, bool>(INDEXA), py::arg("kdata"),
          py::arg("fill_null") = true, R"(INDEXA([kdata])
    
    Return the corresponding market amounts, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index)");

    m.def("INDEXADV", py::overload_cast<>(INDEXADV));
    m.def("INDEXADV", py::overload_cast<const KQuery&>(INDEXADV), R"(INDEXADV([query])
    
    The TDX 880005 market-wide number of the rising stocks, which may not be updated intraday!)");

    m.def("INDEXDEC", py::overload_cast<>(INDEXDEC));
    m.def("INDEXDEC", py::overload_cast<const KQuery&>(INDEXDEC), R"(INDEXDEC([query])
    
    The TDX 880005 market-wide number of the falling stocks, which may not be updated intraday!)");

    m.def("WINNER", py::overload_cast<>(WINNER));
    m.def("WINNER", py::overload_cast<const Indicator&>(WINNER));
    m.def("WINNER", py::overload_cast<Indicator::value_t>(WINNER), R"(WINNER([ind])
    
    The profitable position ratio
    Usage: WINNER(CLOSE) means the ratio of the profitable positions sold at the current closing price.
    For example: returning 0.1 means 10% of the profitable positions; WINNER(10.5) means the ratio of the profitable positions at the price of 10.5 yuan
    This function is only valid for the daily analysis period, and only for the securities with the circulating share capital dividend data; it is invalid for the indexes, the funds, etc.)");

    m.def("INBLOCK", py::overload_cast<const string&, const string&>(INBLOCK), py::arg("category"),
          py::arg("name"));
    m.def("INBLOCK", py::overload_cast<const KData&, const string&, const string&>(INBLOCK),
          py::arg("data"), py::arg("category"), py::arg("name"),
          R"(INBLOCK(data, category, name)        

    Whether the security of the current context is in the specified block.

    :param KData data: the specified K-line data (the context)
    :param string category: the block category
    :param string name: the block name
    :rtype: Indicator)");

    m.def("CODELIKE", py::overload_cast<const string&>(CODELIKE), py::arg("pattern"));
    m.def("CODELIKE", py::overload_cast<const KData&, const string&>(CODELIKE), py::arg("data"),
          py::arg("pattern"),
          R"(CODELIKE(data, pattern)

    Whether the security code of the current context matches the specified pattern.

    :param KData data: the specified K-line data (the context)
    :param string pattern: the matching pattern, supporting the wildcards * and ?
    :rtype: Indicator)");

    m.def("NAMELIKE", py::overload_cast<const string&>(NAMELIKE), py::arg("pattern"));
    m.def("NAMELIKE", py::overload_cast<const KData&, const string&>(NAMELIKE), py::arg("data"),
          py::arg("pattern"),
          R"(NAMELIKE(data, pattern)

    Whether the security name of the current context matches the specified pattern.

    :param KData data: the specified K-line data (the context)
    :param string pattern: the matching pattern, supporting the wildcards * and ?
    :rtype: Indicator)");

    m.def("DISCARD", py::overload_cast<int>(DISCARD), py::arg("discard"));
    m.def("DISCARD", py::overload_cast<const Indicator&, int>(DISCARD), py::arg("ind"),
          py::arg("discard"), R"(DISCARD(ind, discard)
    
    Set the amount of the discarded data of the indicator result in the way of the indicator formula.

    :param Indicator ind: the indicator
    :param int discard: the amount of the discarded data
    :rtype: Indicator)");

    m.def("LASTVALUE", py::overload_cast<bool>(LASTVALUE), py::arg("ignore_discard") = false);
    m.def("LASTVALUE", py::overload_cast<const Indicator&, bool>(LASTVALUE), py::arg("ind"),
          py::arg("ignore_discard") = false, R"(LASTVALUE(ind, [ignore_discard=False])

    Equivalent to the TDX CONST indicator. Take the last value of the input indicator as a constant, i.e. all the values in the result are the last value of the input indicator; use with caution. It contains a look-ahead function; use with caution.

    :param Indicator ind: the indicator
    :param bool ignore_discard: ignore the discarded data of the indicator
    :rtype: Indicator)");

    m.def("JUMPUP", py::overload_cast<>(JUMPUP));
    m.def("JUMPUP", py::overload_cast<const Indicator&>(JUMPUP), R"(JUMPUP([ind])
    
    The edge jump, jumping from <= 0.0 to > 0.0
    
    :param Indicator ind: the indicator
    :rtype: Indicator)");

    m.def("JUMPDOWN", py::overload_cast<>(JUMPDOWN));
    m.def("JUMPDOWN", py::overload_cast<const Indicator&>(JUMPDOWN), R"(JUMPDOWN([ind])
    
    The edge jump, jumping from > 0.0 to <= 0.0

    :param Indicator ind: the indicator
    :rtype: Indicator)");

    m.def("CYCLE", py::overload_cast<int, const string&, bool>(CYCLE), py::arg("adjust_cycle") = 1,
          py::arg("adjust_mode") = "query", py::arg("delay_to_trading_day") = true);
    m.def("CYCLE", py::overload_cast<const KData&, int, const string&, bool>(CYCLE),
          py::arg("kdata"), py::arg("adjust_cycle") = 1, py::arg("adjust_mode") = "query",
          py::arg("delay_to_trading_day") = true,
          R"(CYCLE(kdata, [adjust_cycle=1], [adjust_mode='query'], [delay_to_trading_day=True])
          
    The PF position adjustment period indicator, mainly used for the PF position adjustment day verification, and as an SG

    :param KData kdata: the K-line data
    :param int adjust_cycle: the adjustment period
    :param string adjust_mode: the adjustment way
    :param bool delay_to_trading_day: whether the adjustment period is postponed to the trading day
    :rtype: Indicator)");

    m.def("KALMAN", py::overload_cast<double, double>(KALMAN), py::arg("q") = 0.01,
          py::arg("r") = 0.1);
    m.def("KALMAN", py::overload_cast<const Indicator&, double, double>(KALMAN), py::arg("ind"),
          py::arg("q") = 0.01, py::arg("r") = 0.1, R"(KALMAN(ind, [q=0.01], [r=0.1])

    The Kalman filter, used to smooth the indicator; the smoothing coefficients q and r can be set, defaulting to q=0.01, r=0.1

    :param Indicator ind: the indicator
    :param float q: the smoothing coefficient
    :param float r: the noise coefficient
    :rtype: Indicator)");

    m.def("TR", py::overload_cast<>(TR));
    m.def("TR", py::overload_cast<const KData&>(TR), py::arg("kdata"), R"(TR([kdata])

    The true range (TR) is the maximum of the following three values:
    1. The difference between the high price and the low price of the current period
    2. The absolute value of the difference between the high price of the current period and the close price of the previous period
    3. The absolute value of the difference between the low price of the current period and the close price of the previous period

    :param KData kdata: the K-line data
    :rtype: Indicator)");

    m.def("BARSLASTCOUNT", py::overload_cast<>(BARSLASTCOUNT));
    m.def("BARSLASTCOUNT", py::overload_cast<const Indicator&>(BARSLASTCOUNT), py::arg("data"),
          R"(BARSLASTCOUNT([data])

    Used to count the number of the periods continuously satisfying the condition
    BARSLASTCOUNT(X), where X is a condition expression. For example, BARSLASTCOUNT(CLOSE>OPEN) means counting the number of the periods continuously closing bullish

    :param Indicator data: the condition indicator
    :rtype: Indicator)");

    m.def("REFX", py::overload_cast<int>(REFX), py::arg("n"));
    m.def("REFX", py::overload_cast<const Indicator&, int>(REFX), py::arg("ind"), py::arg("n"),
          R"(REFX(ind, n)

    The enhancement of REF (an unsafe reference, do not use it for backtesting). It is used to get the value of the nth period in the indicator; when n is positive, count forward from the current period; when it is negative, count backward from the current period.

    :param Indicator ind: the indicator
    :param int n: the number of the periods)");

    // The ISLIMITUP limit-up judgment indicator binding
    m.def("ISLIMITUP", ISLIMITUP1, R"(ISLIMITUP()

    The indicator judging whether the stock is the limit up

    Judge whether the close price of the day reaches the limit-up price according to the different stock types:
    - The ordinary A-share stocks: the limit-up range is 10%
    - The Beijing Stock Exchange stocks: the limit-up range is 30%
    - The ChiNext/STAR Market stocks: the limit-up range is 20%
    - The ST stocks have a limit-up range of 5%, but since the historical date information of the ST mark is lacking, it is not handled for now

    The limit-up judgment logic: the close price of the day >= the close price of the previous day × (1 + the limit-up range)

    :rtype: Indicator)");

    m.def("ISLIMITUP", ISLIMITUP2, py::arg("kdata"), R"(ISLIMITUP(kdata)

    Judge whether the stock in the specified K-line data is the limit up

    :param KData kdata: the K-line data
    :rtype: Indicator)");

    // The ISLIMITDOWN limit-down judgment indicator binding
    m.def("ISLIMITDOWN", ISLIMITDOWN1, R"(ISLIMITDOWN()

    The indicator judging whether the stock is the limit down

    Judge whether the close price of the day reaches the limit-down price according to the different stock types:
    - The ordinary A-share stocks: the limit-down range is 10%
    - The Beijing Stock Exchange stocks: the limit-down range is 30%
    - The ChiNext/STAR Market stocks: the limit-down range is 20%
    - The ST stocks have a limit-down range of 5%, but since the historical date information of the ST mark is lacking, it is not handled for now

    The limit-down judgment logic: the close price of the day <= the close price of the previous day × (1 - the limit-down range)

    :rtype: Indicator)");

    m.def("ISLIMITDOWN", ISLIMITDOWN2, py::arg("kdata"), R"(ISLIMITDOWN(kdata)

    Judge whether the stock in the specified K-line data is the limit down

    :param KData kdata: the K-line data
    :rtype: Indicator)");

    m.def("FACTOR", py::overload_cast<const string&>(FACTOR), py::arg("factor"));
    m.def("FACTOR", py::overload_cast<const Factor&>(&FACTOR), py::arg("factor"), R"(FACTOR(factor)
    FACTOR(factor)

    The factor indicator conversion

    Convert a Factor object to an Indicator, so that it can be used in the indicator system.
    This indicator needs to set the K-line context to calculate.

    :param Factor factor: the factor object (choose one of name and it)
    :param str name: the factor name (the convenient version, choose one of factor and it)
    :rtype: Indicator)");
}