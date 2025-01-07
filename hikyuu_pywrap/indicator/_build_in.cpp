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

Indicator (*KDATA_PART1)(const KData& kdata, const string& part) = KDATA_PART;
Indicator (*KDATA_PART3)(const string& part) = KDATA_PART;

Indicator (*RECOVER_FORWARD_1)() = RECOVER_FORWARD;
Indicator (*RECOVER_FORWARD_2)(const Indicator&) = RECOVER_FORWARD;
Indicator (*RECOVER_FORWARD_3)(const KData&) = RECOVER_FORWARD;

Indicator (*RECOVER_BACKWARD_1)() = RECOVER_BACKWARD;
Indicator (*RECOVER_BACKWARD_2)(const Indicator&) = RECOVER_BACKWARD;
Indicator (*RECOVER_BACKWARD_3)(const KData&) = RECOVER_BACKWARD;

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

// 太多选项选择的话，python中无法加载
Indicator (*AMA_1)(int, int, int) = AMA;
Indicator (*AMA_2)(const IndParam&, const IndParam&, const IndParam&) = AMA;
Indicator (*AMA_3)(const Indicator&, int, int, int) = AMA;
Indicator (*AMA_4)(const Indicator&, const IndParam&, const IndParam&, const IndParam&) = AMA;
Indicator (*AMA_5)(const Indicator&, const Indicator&, const Indicator&, const Indicator&) = AMA;

Indicator (*ATR_1)(int) = ATR;
Indicator (*ATR_2)(const IndParam&) = ATR;
Indicator (*ATR_3)(const Indicator&, const IndParam&) = ATR;
Indicator (*ATR_4)(const Indicator&, const Indicator&) = ATR;
Indicator (*ATR_5)(const Indicator&, int) = ATR;

Indicator (*DIFF_1)() = DIFF;
Indicator (*DIFF_2)(const Indicator&) = DIFF;

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

Indicator (*MDD_1)() = MDD;
Indicator (*MDD_2)(const Indicator&) = MDD;

Indicator (*MRR_1)() = MRR;
Indicator (*MRR_2)(const Indicator&) = MRR;

Indicator (*ZHBOND10_1)(double) = ZHBOND10;
Indicator (*ZHBOND10_2)(const DatetimeList&, double) = ZHBOND10;
Indicator (*ZHBOND10_3)(const KData& k, double) = ZHBOND10;
Indicator (*ZHBOND10_4)(const Indicator&, double) = ZHBOND10;

Indicator (*CORR_1)(const Indicator&, int, bool) = CORR;
Indicator (*CORR_2)(const Indicator&, const Indicator&, int, bool) = CORR;

Indicator (*SPEARMAN_1)(const Indicator&, int, bool) = SPEARMAN;
Indicator (*SPEARMAN_2)(const Indicator&, const Indicator&, int, bool) = SPEARMAN;

Indicator (*ZSCORE_1)(bool, double, bool) = ZSCORE;
Indicator (*ZSCORE_2)(const Indicator&, bool, double, bool) = ZSCORE;

void export_Indicator_build_in(py::module& m) {
    m.def("C_KDATA", KDATA1);
    m.def("C_KDATA", KDATA3, R"(KDATA([data])

    包装KData成Indicator，用于其他指标计算

    :param data: KData 或 具有6个返回结果的Indicator（如KDATA生成的Indicator）
    :rtype: Indicator)");

    m.def("C_CLOSE", CLOSE1);
    m.def("C_CLOSE", CLOSE3, R"(CLOSE([data])

    获取收盘价，包装KData的收盘价成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    m.def("C_OPEN", OPEN1);
    m.def("C_OPEN", OPEN3, R"(OPEN([data])

    获取开盘价，包装KData的开盘价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    m.def("C_HIGH", HIGH1);
    m.def("C_HIGH", HIGH3, R"(HIGH([data])

    获取最高价，包装KData的最高价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    m.def("C_LOW", LOW1);
    m.def("C_LOW", LOW3, R"(LOW([data])

    获取最低价，包装KData的最低价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    m.def("C_AMO", AMO1);
    m.def("C_AMO", AMO3, R"(AMO([data])

    获取成交金额，包装KData的成交金额成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    m.def("C_VOL", VOL1);
    m.def("C_VOL", VOL3, R"(VOL([data])

    获取成交量，包装KData的成交量成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    m.def("KDATA_PART", KDATA_PART1, py::arg("data"), py::arg("kpart"));
    m.def("KDATA_PART", KDATA_PART3, py::arg("kpart"), R"(KDATA_PART([data, kpart])

    根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL，如:KDATA_PART("CLOSE")等同于CLOSE()

    :param data: 输入数据（KData 或 Indicator） 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :rtype: Indicator)");

    m.def("RECOVER_FORWARD", RECOVER_FORWARD_1);
    m.def("RECOVER_FORWARD", RECOVER_FORWARD_2);
    m.def("RECOVER_FORWARD", RECOVER_FORWARD_3, R"(RECOVER_FORWARD([data])
    
    对输入的指标数据 (CLOSE|OPEN|HIGH|LOW) 进行前向复权

    :param Indicator|KData data: 只接受 CLOSE|OPEN|HIGH|LOW 指标，或 KData（此时默认使用 KData 的收盘价）
    :rtype: Indicator)");

    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_1);
    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_2);
    m.def("RECOVER_BACKWARD", RECOVER_BACKWARD_3, R"(RECOVER_BACKWARD([data])
    
    对输入的指标数据 (CLOSE|OPEN|HIGH|LOW) 进行后向复权

    :param Indicator|KData data: 只接受 CLOSE|OPEN|HIGH|LOW 指标，或 KData（此时默认使用 KData 的收盘价）
    :rtype: Indicator)");

    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_1);
    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_2);
    m.def("RECOVER_EQUAL_FORWARD", RECOVER_EQUAL_FORWARD_3, R"(RECOVER_EQUAL_FORWARD([data])
    
    对输入的指标数据 (CLOSE|OPEN|HIGH|LOW) 进行等比前向复权

    :param Indicator|KData data: 只接受 CLOSE|OPEN|HIGH|LOW 指标，或 KData（此时默认使用 KData 的收盘价）
    :rtype: Indicator)");

    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_1);
    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_2);
    m.def("RECOVER_EQUAL_BACKWARD", RECOVER_EQUAL_BACKWARD_3, R"(RECOVER_EQUAL_BACKWARD([data])
    
    对输入的指标数据 (CLOSE|OPEN|HIGH|LOW) 进行等比后向复权

    :param Indicator|KData data: 只接受 CLOSE|OPEN|HIGH|LOW 指标，或 KData（此时默认使用 KData 的收盘价）
    :rtype: Indicator)");

    m.def("DATE", DATE1);
    m.def("DATE", DATE2, R"(DATE([data])

    取得该周期从1900以来的年月日。用法: DATE 例如函数返回1000101，表示2000年1月1日。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("TIME", TIME1);
    m.def("TIME", TIME2, R"(TIME([data])

    取得该周期的时分秒。用法: TIME 函数返回有效值范围为(000000-235959)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("YEAR", YEAR1);
    m.def("YEAR", YEAR2, R"(YEAR([data])

    取得该周期的年份。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("MONTH", MONTH1);
    m.def("MONTH", MONTH2, R"(MONTH([data])

    取得该周期的月份。用法: MONTH 函数返回有效值范围为(1-12)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("WEEK", WEEK1);
    m.def("WEEK", WEEK2, R"(WEEK([data])

    取得该周期的星期数。用法：WEEK 函数返回有效值范围为(0-6)，0表示星期天。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("DAY", DAY1);
    m.def("DAY", DAY2, R"(DAY([data])

    取得该周期的日期。用法: DAY 函数返回有效值范围为(1-31)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("HOUR", HOUR1);
    m.def("HOUR", HOUR2, R"(HOUR([data])

    取得该周期的小时数。用法：HOUR 函数返回有效值范围为(0-23)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("MINUTE", MINUTE1);
    m.def("MINUTE", MINUTE2, R"(MINUTE([data])

    取得该周期的分钟数。用法：MINUTE 函数返回有效值范围为(0-59)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    m.def("CONTEXT", py::overload_cast<bool>(hku::CONTEXT), py::arg("fill_null") = true);
    m.def("CONTEXT", py::overload_cast<const Indicator&, bool>(hku::CONTEXT), py::arg("ind"),
          py::arg("fill_null") = true, R"(CONTEXT(ind)
    
    独立上下文。使用 ind 自带的上下文。当指定新的上下文时，不会改变已有的上下文。
    例如：ind = CLOSE(k1), 当指定新的上下文 ind = ind(k2) 时，使用的是 k2 的收盘价。如想仍使用 k1 收盘价，
    则需使用 ind = CONTEXT(CLOSE(k1)), 此时 ind(k2) 将仍旧使用 k1 的收盘价。
    
    :param Indicator ind: 指标对象
    :param bool fill_null: 日期对齐时，缺失日期对应填充空值
    :rtype: Indicator)");

    m.def("CONTEXT_K", CONTEXT_K, R"(CONTEXT_K(ind)

    获取指标上下文。Indicator::getContext()方法获取的是当前的上下文，但对于 CONTEXT 独立上下文指标无法获取其指定的独立上下文，需用此方法获取

    :param Indicator ind: 指标对象
    :rtype: KData)");

    m.def(
      "PRICELIST",
      [](const py::object& obj, int result_index = 0, int discard = 0,
         const py::object& pyalign_dates = py::none()) {
          if (py::isinstance<Indicator>(obj)) {
              Indicator data = obj.cast<Indicator>();
              return PRICELIST(data, result_index);
          } else if (py::isinstance<py::sequence>(obj)) {
              const auto& x = obj.cast<py::sequence>();
              auto values = python_list_to_vector<price_t>(x);
              if (pyalign_dates.is_none()) {
                  return PRICELIST(values, discard);
              } else {
                  py::sequence align_dates = pyalign_dates.cast<py::sequence>();
                  auto total = len(align_dates);
                  DatetimeList dates(total);
                  for (auto i = 0; i < total; ++i) {
                      dates[i] = pydatetime_to_Datetime(align_dates[i]);
                  }
                  return PRICELIST(values, dates, discard);
              }
          }
          HKU_THROW("Invalid input data type!");
      },
      py::arg("data"), py::arg("result_index") = 0, py::arg("discard") = 0,
      py::arg("align_dates") = py::none());

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

    求移动平均

    用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :param float|Indicator|IndParam m: 系数
    :rtype: Indicator)");

    m.def("EMA", EMA_1, py::arg("n") = 22);
    m.def("EMA", EMA_2, py::arg("n"));
    m.def("EMA", EMA_3, py::arg("data"), py::arg("n"));
    m.def("EMA", EMA_4, py::arg("data"), py::arg("n"));
    m.def("EMA", EMA_5, py::arg("data"), py::arg("n") = 22, R"(EMA([data, n=22])

    指数移动平均线(Exponential Moving Average)

    :param data: 输入数据
    :param int|Indicator|IndParam n n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator)");

    m.def("MA", MA_1, py::arg("n") = 22);
    m.def("MA", MA_2, py::arg("n"));
    m.def("MA", MA_3, py::arg("data"), py::arg("n"));
    m.def("MA", MA_4, py::arg("data"), py::arg("n"));
    m.def("MA", MA_5, py::arg("data"), py::arg("n") = 22, R"(MA([data, n=22])

    简单移动平均

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("WMA", WMA_1, py::arg("n") = 22);
    m.def("WMA", WMA_2, py::arg("n"));
    m.def("WMA", WMA_3, py::arg("data"), py::arg("n"));
    m.def("WMA", WMA_4, py::arg("data"), py::arg("n"));
    m.def("WMA", WMA_5, py::arg("data"), py::arg("n") = 22, R"(WMA([data, n=22])

    加权移动平均，算法:Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("AMA", AMA_1, py::arg("n") = 10, py::arg("fast_n") = 2, py::arg("slow_n") = 30);
    m.def("AMA", AMA_2, py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_4, py::arg("data"), py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_5, py::arg("data"), py::arg("n"), py::arg("fast_n"), py::arg("slow_n"));
    m.def("AMA", AMA_3, py::arg("data"), py::arg("n") = 10, py::arg("fast_n") = 2,
          py::arg("slow_n") = 30,
          R"(AMA([data, n=10, fast_n=2, slow_n=30])

    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于2的整数
    :param int|Indicator|IndParam fast_n: 对应快速周期N
    :param int|Indicator|IndParam slow_n: 对应慢速EMA线的N值
    :rtype: Indicator

    * result(0): AMA
    * result(1): ER)");

    m.def("ATR", ATR_1, py::arg("n") = 14);
    m.def("ATR", ATR_2, py::arg("n"));
    m.def("ATR", ATR_3, py::arg("data"), py::arg("n"));
    m.def("ATR", ATR_4, py::arg("data"), py::arg("n"));
    m.def("ATR", ATR_5, py::arg("data"), py::arg("n") = 14, R"(ATR([data, n=14])

    平均真实波幅(Average True Range)

    :param Indicator data 待计算的源数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于1的整数
    :rtype: Indicator)");

    m.def("MACD", MACD_1, py::arg("n1") = 12, py::arg("n2") = 26, py::arg("n3") = 9);
    m.def("MACD", MACD_2, py::arg("n1"), py::arg("n2"), py::arg("n3"));
    m.def("MACD", MACD_3, py::arg("data"), py::arg("n1") = 12, py::arg("n2") = 26,
          py::arg("n3") = 9);
    m.def("MACD", MACD_4, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("n3"));
    m.def("MACD", MACD_5, py::arg("data"), py::arg("n1"), py::arg("n2"), py::arg("n3"),
          R"(MACD([data, n1=12, n2=26, n3=9])

    平滑异同移动平均线

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n1: 短期EMA时间窗
    :param int|Indicator|IndParam n2: 长期EMA时间窗
    :param int|Indicator|IndParam n3: （短期EMA-长期EMA）EMA平滑时间窗
    :rtype: 具有三个结果集的 Indicator

    * result(0): MACD_BAR：MACD直柱，即MACD快线－MACD慢线
    * result(1): DIFF: 快线,即（短期EMA-长期EMA）
    * result(2): DEA: 慢线，即快线的n3周期EMA平滑)");

    m.def("VIGOR", VIGOR_1, py::arg("kdata"), py::arg("n") = 2);
    m.def("VIGOR", VIGOR_2, py::arg("n") = 2, R"(VIGOR([kdata, n=2])

    亚历山大.艾尔德力度指数 [BOOK2]_

    计算公式：（收盘价今－收盘价昨）＊成交量今

    :param KData data: 输入数据
    :param int n: EMA平滑窗口
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

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_

    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n1: 计算平均噪音的回溯时间窗口
    :param int|Indicator|IndParam n2: 对初步止损线去n2日内的最高值
    :param float|Indicator|IndParam p: 噪音系数
    :rtype: Indicator)");

    m.def("DIFF", DIFF_1);
    m.def("DIFF", DIFF_2, R"(DIFF([data])

    差分指标，即data[i] - data[i-1]

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("REF", REF_1, py::arg("n"));
    m.def("REF", REF_2, py::arg("n"));
    m.def("REF", REF_3, py::arg("data"), py::arg("n"));
    m.def("REF", REF_4, py::arg("data"), py::arg("n"));
    m.def("REF", REF_5, py::arg("data"), py::arg("n"), R"(REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。

    用法：REF(X，A)　引用A周期前的X值。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 引用n周期前的值，即右移n位
    :rtype: Indicator)");

    m.def("STDEV", STDEV_1, py::arg("n") = 10);
    m.def("STDEV", STDEV_2, py::arg("n"));
    m.def("STDEV", STDEV_3, py::arg("data"), py::arg("n"));
    m.def("STDEV", STDEV_4, py::arg("data"), py::arg("n"));
    m.def("STDEV", STDEV_5, py::arg("data"), py::arg("n") = 10, R"(STDEV([data, n=10])

    计算N周期内样本标准差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("STDP", STDP_1, py::arg("n") = 10);
    m.def("STDP", STDP_2, py::arg("n"));
    m.def("STDP", STDP_3, py::arg("data"), py::arg("n"));
    m.def("STDP", STDP_4, py::arg("data"), py::arg("n"));
    m.def("STDP", STDP_5, py::arg("data"), py::arg("n") = 10, R"(STDP([data, n=10])

    总体标准差，STDP(X,N)为X的N日总体标准差

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    m.def("POS", POS, py::arg("block"), py::arg("query"), py::arg("sg"));

    m.def("HHV", HHV_1, py::arg("n") = 20);
    m.def("HHV", HHV_2, py::arg("n"));
    m.def("HHV", HHV_3, py::arg("data"), py::arg("n"));
    m.def("HHV", HHV_4, py::arg("data"), py::arg("n"));
    m.def("HHV", HHV_5, py::arg("data"), py::arg("n") = 20, R"(HHV([data, n=20])

    N日内最高价，N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    m.def("LLV", LLV_1, py::arg("n") = 20);
    m.def("LLV", LLV_2, py::arg("n"));
    m.def("LLV", LLV_3, py::arg("data"), py::arg("n"));
    m.def("LLV", LLV_4, py::arg("data"), py::arg("n"));
    m.def("LLV", LLV_5, py::arg("data"), py::arg("n") = 20, R"(LLV([data, n=20])

    N日内最低价，N=0则从第一个有效值开始。

    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    m.def("CVAL", CVAL_1, py::arg("value") = 0.0, py::arg("discard") = 0);
    m.def("CVAL", CVAL_2, py::arg("data"), py::arg("value") = 0.0, py::arg("discard") = 0,
          R"(CVAL([data, value=0.0, discard=0])

    data 为 Indicator 实例，创建和 data 等长的常量指标，其值和为value，抛弃长度discard和data一样

    :param Indicator data: Indicator实例
    :param float value: 常数值
    :param int discard: 抛弃数量
    :rtype: Indicator)");

    m.def("LIUTONGPAN", LIUTONGPAN_1);
    m.def("LIUTONGPAN", LIUTONGPAN_2, R"(LIUTONGPAN(kdata)

   获取流通盘（单位：万股），同 CAPITAL

   :param KData kdata: k线数据
   :rtype: Indicator)");

    m.def("ZONGGUBEN", py::overload_cast<>(ZONGGUBEN));
    m.def("ZONGGUBEN", py::overload_cast<const KData&>(ZONGGUBEN), R"(ZONGGUBEN(kdata)

   获取总股本（单位：万股）

   :param KData kdata: k线数据
   :rtype: Indicator)");

    m.def("HSL", HSL_1);
    m.def("HSL", HSL_2, R"(HSL(kdata)

    获取换手率，等于 VOL(k) / CAPITAL(k)

    :param KData kdata: k线数据
    :rtype: Indicator)");

    m.def("WEAVE", WEAVE, R"(WEAVE(ind1, ind2)

    将ind1和ind2的结果组合在一起放在一个Indicator中。如ind = WEAVE(ind1, ind2), 则此时ind包含多个结果，按ind1、ind2的顺序存放。
    
    :param Indicator ind1: 指标1
    :param Indicator ind2: 指标2
    :rtype: Indicator)");

    m.def("CORR", CORR_1, py::arg("ref_ind"), py::arg("n") = 10, py::arg("fill_null") = true);
    m.def("CORR", CORR_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 10,
          py::arg("fill_null") = true,
          R"(CORR(ind, ref_ind[, n=10, fill_null=True])

    计算 ind 和 ref_ind 的相关系数。返回中存在两个结果，第一个为相关系数，第二个为协方差。
    与 CORR(ref_ind, n)(ind) 等效。

    :param Indicator ind: 指标1
    :param Indicator ref_ind: 指标2
    :param int n: 按指定 n 的长度计算两个 ind 直接数据相关系数。如果为0，使用输入的ind长度。
    :param bool fill_null: 日期对齐时缺失日期填充nan值
    :rtype: Indicator)");

    m.def("IF", IF_1);
    m.def("IF", IF_2);
    m.def("IF", IF_3);
    m.def("IF", IF_4, R"(IF(x, a, b)

    条件函数, 根据条件求不同的值。

    用法：IF(X,A,B)若X不为0则返回A,否则返回B

    例如：IF(CLOSE>OPEN,HIGH,LOW)表示该周期收阳则返回最高值,否则返回最低值

    :param Indicator x: 条件指标
    :param Indicator a: 待选指标 a
    :param Indicator b: 待选指标 b
    :rtype: Indicator)");

    m.def("COUNT", COUNT_1, py::arg("n") = 20);
    m.def("COUNT", COUNT_2, py::arg("n"));
    m.def("COUNT", COUNT_3, py::arg("data"), py::arg("n"));
    m.def("COUNT", COUNT_4, py::arg("data"), py::arg("n"));
    m.def("COUNT", COUNT_5, py::arg("data"), py::arg("n") = 20, R"(COUNT([data, n=20])

    统计满足条件的周期数。

    用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。

    例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数

    :param Indicator data: 条件
    :param int|Indicator|IndParam n: 周期
    :rtype: Indicator)");

    m.def("SUM", SUM_1, py::arg("n") = 20);
    m.def("SUM", SUM_2, py::arg("n"));
    m.def("SUM", SUM_3, py::arg("data"), py::arg("n"));
    m.def("SUM", SUM_4, py::arg("data"), py::arg("n"));
    m.def("SUM", SUM_5, py::arg("data"), py::arg("n") = 20, R"(SUM([data, n=20])

    求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("ABS", ABS_1);
    m.def("ABS", ABS_2);
    m.def("ABS", ABS_3, R"(ABS([data])

    求绝对值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("NOT", NOT_1);
    m.def("NOT", NOT_2, R"(NOT([data])

    求逻辑非。NOT(X)返回非X,即当X=0时返回1，否则返回0。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("SGN", SGN_1);
    m.def("SGN", SGN_2);
    m.def("SGN", SGN_3, R"(SGN([data])

    求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("EXP", EXP_1);
    m.def("EXP", EXP_2);
    m.def("EXP", EXP_3, R"(EXP([data])

    EXP(X)为e的X次幂

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("MAX", MAX_1);
    m.def("MAX", MAX_2);
    m.def("MAX", MAX_3, R"(MAX(ind1, ind2)

    求最大值, MAX(A,B)返回A和B中的较大值。

    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator)");

    m.def("MIN", MIN_1);
    m.def("MIN", MIN_2);
    m.def("MIN", MIN_3, R"(MIN(ind1, ind2)

    求最小值, MIN(A,B)返回A和B中的较小值。

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

    介于(介于两个数之间)

    用法：BETWEEN(A,B,C)表示A处于B和C之间时返回1，否则返回0

    例如：BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5))表示收盘价介于5日均线和10日均线之间

    :param Indicator a: A
    :param Indicator b: B
    :param Indicator c: C
    :rtype: Indicator)");

    m.def("LN", LN_1);
    m.def("LN", LN_2);
    m.def("LN", LN_3, R"(LN([data])

    求自然对数, LN(X)以e为底的对数

    :param data: 输入数据
    :rtype: Indicator)");

    m.def("LOG", LOG_1);
    m.def("LOG", LOG_2);
    m.def("LOG", LOG_3, R"(LOG([data])

    以10为底的对数

    :param data: 输入数据
    :rtype: Indicator)");

    m.def("HHVBARS", HHVBARS_1, py::arg("n") = 20);
    m.def("HHVBARS", HHVBARS_2, py::arg("n"));
    m.def("HHVBARS", HHVBARS_3, py::arg("data"), py::arg("n"));
    m.def("HHVBARS", HHVBARS_4, py::arg("data"), py::arg("n"));
    m.def("HHVBARS", HHVBARS_5, py::arg("data"), py::arg("n") = 20, R"(HHVBARS([data, n=20])

    上一高点位置 求上一高点到当前的周期数。

    用法：HHVBARS(X,N):求N周期内X最高值到当前周期数N=0表示从第一个有效值开始统计

    例如：HHVBARS(HIGH,0)求得历史新高到到当前的周期数

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    m.def("LLVBARS", LLVBARS_1, py::arg("n") = 20);
    m.def("LLVBARS", LLVBARS_2, py::arg("n"));
    m.def("LLVBARS", LLVBARS_3, py::arg("data"), py::arg("n"));
    m.def("LLVBARS", LLVBARS_4, py::arg("data"), py::arg("n"));
    m.def("LLVBARS", LLVBARS_5, py::arg("data"), py::arg("n") = 20, R"(LLVBARS([data, n=20])

    上一低点位置 求上一低点到当前的周期数。

    用法：LLVBARS(X,N):求N周期内X最低值到当前周期数N=0表示从第一个有效值开始统计

    例如：LLVBARS(HIGH,20)求得20日最低点到当前的周期数

    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    m.def("POW", POW_1, py::arg("n"));
    m.def("POW", POW_2, py::arg("n"));
    m.def("POW", POW_3, py::arg("data"), py::arg("n"));
    m.def("POW", POW_4, py::arg("data"), py::arg("n"));
    m.def("POW", POW_5, py::arg("data"), py::arg("n"));
    m.def("POW", POW_6, py::arg("data"), py::arg("n"), R"(POW(data, n)

    乘幂

    用法：POW(A,B)返回A的B次幂

    例如：POW(CLOSE,3)求得收盘价的3次方

    :param data: 输入数据
    :param int|Indicator|IndParam n: 幂
    :rtype: Indicator)");

    m.def("SQRT", SQRT_1);
    m.def("SQRT", SQRT_2);
    m.def("SQRT", SQRT_3, R"(SQRT([data])

    开平方

    用法：SQRT(X)为X的平方根

    例如：SQRT(CLOSE)收盘价的平方根

    :param data: 输入数据
    :rtype: Indicator)");

    m.def("ROUND", ROUND_1, py::arg("ndigits") = 2);
    m.def("ROUND", ROUND_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUND", ROUND_3, py::arg("data"), py::arg("ndigits") = 2, R"(ROUND([data, ndigits=2])

    四舍五入

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    m.def("ROUNDUP", ROUNDUP_1, py::arg("ndigits") = 2);
    m.def("ROUNDUP", ROUNDUP_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUNDUP", ROUNDUP_3, py::arg("data"), py::arg("ndigits") = 2,
          R"(ROUNDUP([data, ndigits=2])

    向上截取，如10.1截取后为11

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    m.def("ROUNDDOWN", ROUNDDOWN_1, py::arg("ndigits") = 2);
    m.def("ROUNDDOWN", ROUNDDOWN_2, py::arg("data"), py::arg("ndigits") = 2);
    m.def("ROUNDDOWN", ROUNDDOWN_3, py::arg("data"), py::arg("ndigits") = 2,
          R"(ROUND([data, ndigits=2])

    四舍五入

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    m.def("FLOOR", FLOOR_1);
    m.def("FLOOR", FLOOR_2);
    m.def("FLOOR", FLOOR_3, R"(FLOOR([data])

    向下舍入(向数值减小方向舍入)取整

    用法：FLOOR(A)返回沿A数值减小方向最接近的整数

    例如：FLOOR(12.3)求得12

    :param data: 输入数据
    :rtype: Indicator)");

    m.def("CEILING", CEILING_1);
    m.def("CEILING", CEILING_2);
    m.def("CEILING", CEILING_3, R"(CEILING([data])

    向上舍入(向数值增大方向舍入)取整
   
    用法：CEILING(A)返回沿A数值增大方向最接近的整数
   
    例如：CEILING(12.3)求得13；CEILING(-3.5)求得-3
   
    :param data: 输入数据
    :rtype: Indicator)");

    m.def("INTPART", INTPART_1);
    m.def("INTPART", INTPART_2);
    m.def("INTPART", INTPART_3, R"(INTPART([data])

    取整(绝对值减小取整，即取得数据的整数部分)

    :param data: 输入数据
    :rtype: Indicator)");

    m.def("EXIST", EXIST_1, py::arg("n") = 20);
    m.def("EXIST", EXIST_2, py::arg("n"));
    m.def("EXIST", EXIST_3, py::arg("data"), py::arg("n"));
    m.def("EXIST", EXIST_4, py::arg("data"), py::arg("n"));
    m.def("EXIST", EXIST_5, py::arg("data"), py::arg("n") = 20, R"(EXIST([data, n=20])

    存在, EXIST(X,N) 表示条件X在N周期有存在

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator)");

    m.def("EVERY", EVERY_1, py::arg("n") = 20);
    m.def("EVERY", EVERY_2, py::arg("n"));
    m.def("EVERY", EVERY_3, py::arg("data"), py::arg("n"));
    m.def("EVERY", EVERY_4, py::arg("data"), py::arg("n"));
    m.def("EVERY", EVERY_5, py::arg("data"), py::arg("n") = 20, R"(EVERY([data, n=20])

    一直存在

    用法：EVERY (X,N) 表示条件X在N周期一直存在

    例如：EVERY(CLOSE>OPEN,10) 表示前10日内一直是阳线

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
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

    区间存在。

    用法：LAST (X,M,N) 表示条件 X 在前 M 周期到前 N 周期存在。

    例如：LAST(CLOSE>OPEN,10,5) 表示从前10日到前5日内一直阳线。

    :param data: 输入数据
    :param int|Indicator|IndParam m: m周期
    :param int|Indicator|IndParam n: n周期
    :rtype: Indicator)");

    m.def("SIN", SIN_1);
    m.def("SIN", SIN_2);
    m.def("SIN", SIN_3, R"(SIN([data])

    正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("ASIN", ASIN_1);
    m.def("ASIN", ASIN_2);
    m.def("ASIN", ASIN_3, R"(ASIN([data])

    反正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("COS", COS_1);
    m.def("COS", COS_2);
    m.def("COS", COS_3, R"(COS([data])

    余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("ACOS", ACOS_1);
    m.def("ACOS", ACOS_2);
    m.def("ACOS", ACOS_3, R"(ACOS([data])

    反余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("TAN", TAN_1);
    m.def("TAN", TAN_2);
    m.def("TAN", TAN_3, R"(TAN([data])

    正切值

    :param Indicator data: 输入数据
    :rtype: Indicato)");

    m.def("ATAN", ATAN_1);
    m.def("ATAN", ATAN_2);
    m.def("ATAN", ATAN_3, R"(ATAN([data])

    反正切值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("REVERSE", REVERSE_1);
    m.def("REVERSE", REVERSE_2);
    m.def("REVERSE", REVERSE_3, R"(REVERSE([data])

    求相反数，REVERSE(X)返回-X

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("MOD", MOD_1);
    m.def("MOD", MOD_2);
    m.def("MOD", MOD_3);
    m.def("MOD", MOD_4, R"(MOD(ind1, ind2)

    取整后求模。该函数仅为兼容通达信。实际上，指标求模可直接使用 % 操作符

    用法：MOD(A,B)返回A对B求模

    例如：MOD(26,10) 返回 6

    :param Indicator ind1:
    :param Indicator ind2:
    :rtype: Indicator)");

    m.def("VAR", VAR_1, py::arg("n") = 10);
    m.def("VAR", VAR_2, py::arg("n"));
    m.def("VAR", VAR_3, py::arg("data"), py::arg("n"));
    m.def("VAR", VAR_4, py::arg("data"), py::arg("n"));
    m.def("VAR", VAR_5, py::arg("data"), py::arg("n") = 10, R"(VAR([data, n=10])

    估算样本方差, VAR(X,N)为X的N日估算样本方差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("VARP", VARP_1, py::arg("n") = 10);
    m.def("VARP", VARP_2, py::arg("n"));
    m.def("VARP", VARP_3, py::arg("data"), py::arg("n"));
    m.def("VARP", VARP_4, py::arg("data"), py::arg("n"));
    m.def("VARP", VARP_5, py::arg("data"), py::arg("n") = 10, R"(VARP([data, n=10])

    总体样本方差, VARP(X,N)为X的N日总体样本方差

    :param Indicator data: 输入数据
    :param int n|Indicator|IndParam: 时间窗口
    :rtype: Indicator)");

    m.def("UPNDAY", UPNDAY_1, py::arg("data"), py::arg("n") = 3);
    m.def("UPNDAY", UPNDAY_2, py::arg("data"), py::arg("n"));
    m.def("UPNDAY", UPNDAY_3, py::arg("data"), py::arg("n"), R"(UPNDAY(data[, n=3])

    连涨周期数, UPNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("DOWNNDAY", DOWNNDAY_1, py::arg("data"), py::arg("n") = 3);
    m.def("DOWNNDAY", DOWNNDAY_2, py::arg("data"), py::arg("n"));
    m.def("DOWNNDAY", DOWNNDAY_3, py::arg("data"), py::arg("n"), R"(DOWNNDAY(data[, n=3])

    连跌周期数, DOWNNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("NDAY", NDAY_1, py::arg("x"), py::arg("y"), py::arg("n") = 3);
    m.def("NDAY", NDAY_1, py::arg("x"), py::arg("y"), py::arg("n"));
    m.def("NDAY", NDAY_3, py::arg("x"), py::arg("y"), py::arg("n"), R"(NDAY(x, y[, n=3])

    连大, NDAY(X,Y,N)表示条件X>Y持续存在N个周期

    :param Indicator x:
    :param Indicator y:
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("CROSS", CROSS_1);
    m.def("CROSS", CROSS_2);
    m.def("CROSS", CROSS_3);
    m.def("CROSS", CROSS_4, R"(CROSS(x, y)

    交叉函数

    :param x: 变量或常量，判断交叉的第一条线
    :param y: 变量或常量，判断交叉的第二条线
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

    两条线维持一定周期后交叉

    用法：LONGCROSS(A,B,N)表示A在N周期内都小于B，本周期从下方向上穿过B时返 回1，否则返回0

    例如：LONGCROSS(MA(CLOSE,5),MA(CLOSE,10),5)表示5日均线维持5周期后与10日均线交金叉

    :param Indicator a:
    :param Indicator b:
    :param int|Indicator n:
    :rtype: Indicator)");

    m.def("FILTER", FILTER_1, py::arg("n") = 5);
    m.def("FILTER", FILTER_2, py::arg("n"));
    m.def("FILTER", FILTER_3, py::arg("data"), py::arg("n"));
    m.def("FILTER", FILTER_4, py::arg("data"), py::arg("n"));
    m.def("FILTER", FILTER_5, py::arg("data"), py::arg("n") = 5, R"(FILTER([data, n=5])

    信号过滤, 过滤连续出现的信号。

    用法：FILTER(X,N): X 满足条件后，删除其后 N 周期内的数据置为 0。

    例如：FILTER(CLOSE>OPEN,5) 查找阳线，5 天内再次出现的阳线不被记录在内。

    :param Indicator data: 输入数据
    :param int|Indicaot|IndParam n: 过滤周期
    :rtype: Indicator)");

    m.def("BARSSINCE", BARSSINCE_1);
    m.def("BARSSINCE", BARSSINCE_2);
    m.def("BARSSINCE", BARSSINCE_3, R"(BARSSINCE([data])

    第一个条件成立位置到当前的周期数。

    用法：BARSSINCE(X):第一次X不为0到现在的天数。

    例如：BARSSINCE(HIGH>10)表示股价超过10元时到当前的周期数

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("BARSLAST", BARSLAST_1);
    m.def("BARSLAST", BARSLAST_2);
    m.def("BARSLAST", BARSLAST_3, R"(BARSLAST([data])

    上一次条件成立位置 上一次条件成立到当前的周期数。

    用法：BARSLAST(X): 上一次 X 不为 0 到现在的天数。

    例如：BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) 表示上一个涨停板到当前的周期数

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("SUMBARS", SUMBARS_1, py::arg("a"));
    m.def("SUMBARS", SUMBARS_2, py::arg("a"));
    m.def("SUMBARS", SUMBARS_3, py::arg("data"), py::arg("a"));
    m.def("SUMBARS", SUMBARS_4, py::arg("data"), py::arg("a"));
    m.def("SUMBARS", SUMBARS_5, py::arg("data"), py::arg("a"), R"(SUMBARS([data,] a)

    累加到指定周期数, 向前累加到指定值到现在的周期数

    用法：SUMBARS(X,A):将X向前累加直到大于等于A,返回这个区间的周期数

    例如：SUMBARS(VOL,CAPITAL)求完全换手到现在的周期数

    :param Indicator data: 输入数据
    :param float a|Indicator|IndParam: 指定累加和
    :rtype: Indicator)");

    m.def("BARSCOUNT", BARSCOUNT_1);
    m.def("BARSCOUNT", BARSCOUNT_2, R"(BARSCOUNT([data])

    有效值周期数, 求总的周期数。

    用法：BARSCOUNT(X)第一个有效数据到当前的天数。

    例如：BARSCOUNT(CLOSE)对于日线数据取得上市以来总交易日数，对于1分钟线取得当日交易分钟数。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("BACKSET", BACKSET_1, py::arg("n") = 2);
    m.def("BACKSET", BACKSET_2, py::arg("n"));
    m.def("BACKSET", BACKSET_3, py::arg("data"), py::arg("n"));
    m.def("BACKSET", BACKSET_4, py::arg("data"), py::arg("n"));
    m.def("BACKSET", BACKSET_5, py::arg("data"), py::arg("n") = 2, R"(BACKSET([data, n=2])

    向前赋值将当前位置到若干周期前的数据设为1。

    用法：BACKSET(X,N),X非0,则将当前位置到N周期前的数值设为1。

    例如：BACKSET(CLOSE>OPEN,2)若收阳则将该周期及前一周期数值设为1,否则为0

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N周期
    :rtype: Indicator)");

    m.def("TIMELINE", TIMELINE_1);
    m.def("TIMELINE", TIMELINE_2, R"(TIMELINE([k])

    分时价格数据

    :param KData k: 上下文
    :rtype: Indicator)");

    m.def("TIMELINEVOL", TIMELINEVOL_1);
    m.def("TIMELINEVOL", TIMELINEVOL_2, R"(TIMELINEVOL([k])

    分时成交量数据

    :param KData k: 上下文
    :rtype: Indicator)");

    m.def("DMA", DMA, R"(DMA(ind, a[, fill_null=True])

    动态移动平均

    用法：DMA(X,A),求X的动态移动平均。

    算法：若Y=DMA(X,A) 则 Y=A*X+(1-A)*Y',其中Y'表示上一周期Y值。

    例如：DMA(CLOSE,VOL/CAPITAL)表示求以换手率作平滑因子的平均价

    :param Indicator ind: 输入数据
    :param Indicator a: 动态系数
    :param bool fill_null: 日期对齐时缺失数据填充 nan 值。
    :rtype: Indicator)");

    m.def("AVEDEV", AVEDEV_1, py::arg("data"), py::arg("n") = 22);
    m.def("AVEDEV", AVEDEV_2, py::arg("data"), py::arg("n"));
    m.def("AVEDEV", AVEDEV_3, py::arg("data"), py::arg("n"), R"(AVEDEV(data[, n=22])

    平均绝对偏差，求X的N日平均绝对偏差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("DEVSQ", DEVSQ_1, py::arg("n") = 10);
    m.def("DEVSQ", DEVSQ_2, py::arg("n"));
    m.def("DEVSQ", DEVSQ_3, py::arg("data"), py::arg("n"));
    m.def("DEVSQ", DEVSQ_4, py::arg("data"), py::arg("n"));
    m.def("DEVSQ", DEVSQ_5, py::arg("data"), py::arg("n") = 10, R"(DEVSQ([data, n=10])

    数据偏差平方和，求X的N日数据偏差平方和

    :param Indicator data: 输入数据
    :param int|Indicator n: 时间窗口
    :rtype: Indicator)");

    m.def("ROC", ROC_1, py::arg("n") = 10);
    m.def("ROC", ROC_2, py::arg("n"));
    m.def("ROC", ROC_3, py::arg("data"), py::arg("n"));
    m.def("ROC", ROC_4, py::arg("data"), py::arg("n"));
    m.def("ROC", ROC_5, py::arg("data"), py::arg("n") = 10, R"(ROC([data, n=10])

    变动率指标: ((price / prevPrice)-1)*100

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    m.def("ROCP", ROCP_1, py::arg("n") = 10);
    m.def("ROCP", ROCP_2, py::arg("n"));
    m.def("ROCP", ROCP_3, py::arg("data"), py::arg("n"));
    m.def("ROCP", ROCP_4, py::arg("data"), py::arg("n"));
    m.def("ROCP", ROCP_5, py::arg("data"), py::arg("n") = 10, R"(ROCP([data, n=10])

    变动率指标: (price - prevPrice) / prevPrice

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    m.def("ROCR", ROCR_1, py::arg("n") = 10);
    m.def("ROCR", ROCR_2, py::arg("n"));
    m.def("ROCR", ROCR_3, py::arg("data"), py::arg("n"));
    m.def("ROCR", ROCR_4, py::arg("data"), py::arg("n"));
    m.def("ROCR", ROCR_5, py::arg("data"), py::arg("n") = 10, R"(ROCR([data, n=10])

    变动率指标: (price / prevPrice)

    :param data: 输入数据
    :param int n|Indicator|IndParam: 时间窗口
    :rtype: Indicator)");

    m.def("ROCR100", ROCR100_1, py::arg("n") = 10);
    m.def("ROCR100", ROCR100_2, py::arg("n"));
    m.def("ROCR100", ROCR100_3, py::arg("data"), py::arg("n"));
    m.def("ROCR100", ROCR100_4, py::arg("data"), py::arg("n"));
    m.def("ROCR100", ROCR100_5, py::arg("data"), py::arg("n") = 10, R"(ROCR100([data, n=10])

    变动率指标: (price / prevPrice) * 100

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("AD", AD_1);
    m.def("AD", AD_2, R"(AD(kdata)

   累积/派发线

   :param KData kdata: k线数据
   :rtype: Indicator)");

    m.def("COST", COST_1, py::arg("x") = 10.0);
    m.def("COST", COST_2, py::arg("k"), py::arg("x") = 10.0, R"(COST(k[, x=10.0])

    成本分布

    用法：COST(k, X) 表示X%获利盘的价格是多少

    例如：COST(k, 10),表示10%获利盘的价格是多少，即有10%的持仓量在该价格以下，其余90%在该价格以上，为套牢盘 该函数仅对日线分析周期有效

    :param KData k: 关联的K线数据
    :param float x: x%获利价格, 0~100
    :rtype: Indicator)");

    m.def("ALIGN", ALIGN_1, py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_2, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_3, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true);
    m.def("ALIGN", ALIGN_4, py::arg("data"), py::arg("ref"), py::arg("fill_null") = true,
          R"(ALIGN(data, ref):

    按指定的参考日期对齐

    :param Indicator data: 输入数据
    :param DatetimeList|Indicator|KData ref: 指定做为日期参考的 DatetimeList、Indicator 或 KData
    :param bool fill_null: 缺失数据使用 nan 填充; 否则使用小于对应日期且最接近对应日期的数据
    :retype: Indicator)");

    m.def("DROPNA", DROPNA_1);
    m.def("DROPNA", DROPNA_2, R"(DROPNA([data])

    删除 nan 值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    m.def("ADVANCE", ADVANCE, py::arg("query") = KQueryByIndex(-100), py::arg("market") = "SH",
          py::arg("stk_type") = STOCKTYPE_A, py::arg("ignore_context") = false,
          py::arg("fill_null") = true,
          R"(ADVANCE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    上涨家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :para. bool fill_null: 缺失数据使用 nan 填充; 否则使用小于对应日期且最接近对应日期的数据
    :rtype: Indicator)");

    m.def("DECLINE", DECLINE, py::arg("query") = KQueryByIndex(-100), py::arg("market") = "SH",
          py::arg("stk_type") = STOCKTYPE_A, py::arg("ignore_context") = false,
          py::arg("fill_null") = true,
          R"(DECLINE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    下跌家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :param bool fill_null: 缺失数据使用 nan 填充; 否则使用小于对应日期且最接近对应日期的数据
    :rtype: Indicator)");

    m.def("SLICE", SLICE_1, py::arg("data"), py::arg("start"), py::arg("end"));
    m.def("SLICE", SLICE_2, py::arg("start"), py::arg("end"), py::arg("result_index") = 0);
    m.def("SLICE", SLICE_3, py::arg("data"), py::arg("start"), py::arg("end"),
          py::arg("result_index") = 0,
          R"(SLICE(data, start, end, result_index=0)

    获取某指标中指定范围 [start, end) 的数据，生成新的指标

    :param Indicator|PriceList data: 输入数据
    :param int start: 起始位置
    :param int end: 终止位置（不包含本身）
    :param int result_index: 原输入数据中的结果集)");

    m.def("RSI", RSI_1, py::arg("n") = 14);
    m.def("RSI", RSI_2, py::arg("data"), py::arg("n") = 14, R"(RSI([data, n=14])

    相对强弱指数

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("SLOPE", SLOPE1, py::arg("n") = 22);
    m.def("SLOPE", SLOPE2, py::arg("n"));
    m.def("SLOPE", SLOPE3, py::arg("data"), py::arg("n") = 22);
    m.def("SLOPE", SLOPE4, py::arg("data"), py::arg("n"));
    m.def("SLOPE", SLOPE5, py::arg("data"), py::arg("n"), R"(SLOPE([data, n=22])

    计算线性回归斜率，N支持变量

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    m.def("MDD", MDD_1);
    m.def("MDD", MDD_2, R"(MDD([data])
    
    当前价格相对历史最高值的回撤百分比，通常用于计算最大回撤)");

    m.def("MRR", MRR_1);
    m.def("MRR", MRR_2, R"(MRR([data])
    
    当前价格相对历史最低值的盈利百分比，可用于计算历史最高盈利比例)");

    m.def("ZHBOND10", ZHBOND10_1, py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_2, py::arg("data"), py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_3, py::arg("data"), py::arg("default_val") = 0.4);
    m.def("ZHBOND10", ZHBOND10_4, py::arg("data"), py::arg("default_val") = 0.4,
          R"(ZHBOND10([data, defaut_val])

    获取10年期中国国债收益率

    :param DatetimeList|KDate|Indicator data: 输入的日期参考，优先使用上下文中的日期
    :param float default_val: 如果输入的日期早于已有国债数据的最早记录，则使用此默认值)");

    m.def("SPEARMAN", SPEARMAN_1, py::arg("ref_ind"), py::arg("n") = 0,
          py::arg("fill_null") = true);
    m.def("SPEARMAN", SPEARMAN_2, py::arg("ind"), py::arg("ref_ind"), py::arg("n") = 0,
          py::arg("fill_null") = true,
          R"(SPEARMAN(ind, ref_ind[, n=0, fill_null=True])

    Spearman 相关系数。与 SPEARMAN(ref_ind, n)(ind) 等效。

    :param Indicator ind: 输入参数1
    :param Indicator ref_ind: 输入参数2
    :param int n: 滚动窗口(大于2 或 等于0)，等于0时，代表 n 实际使用 ind 的长度
    :param bool fill_null: 缺失数据使用 nan 填充; 否则使用小于对应日期且最接近对应日期的数据)");

    // IR(const Indicator& p, const Indicator& b, int n = 100)
    m.def("IR", IR, py::arg("p"), py::arg("b"), py::arg("n") = 100, R"(IR(p, b[, n])

    信息比率（Information Ratio，IR）

    公式: (P-B) / TE
    P: 组合收益率
    B: 比较基准收益率
    TE: 投资周期中每天的 p 和 b 之间的标准差
    实际使用时，P 一般为 TM 的资产曲线，B 为沪深 3000 收盘价，如:
    ref_k = sm["sh000300"].get_kdata(query)
    funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
    ir = IR(PRICELIST(funds), ref_k.close, 0)

    :param Indicator p:
    :param Indicator b:
    :param int n: 时间窗口，如果只想使用最后的值，可以使用 0, 或 len(p),len(b) 指定
    )");

    m.def(
      "IC",
      [](const Indicator& ind, const py::object& stks, const KQuery& query, const Stock& ref_stk,
         int n, bool spearman) {
          if (py::isinstance<Block>(stks)) {
              const auto& blk = stks.cast<Block&>();
              return IC(ind, blk, query, ref_stk, n, spearman);
          }

          if (py::isinstance<py::sequence>(stks)) {
              StockList c_stks = python_list_to_vector<Stock>(stks);
              return IC(ind, c_stks, query, ref_stk, n, spearman);
          }

          HKU_THROW("Input stks must be Block or sequenc(Stock)!");
      },
      py::arg("ind"), py::arg("stks"), py::arg("query"), py::arg("ref_stk"), py::arg("n") = 1,
      py::arg("spearman") = true,
      R"(IC(ind, stks, query, ref_stk[, n=1])

    计算指定的因子相对于参考证券的 IC （实际为 RankIC）
    
    :param Indicator ind: 输入因子
    :param sequence(stock)|Block stks 证券组合
    :param Query query: 查询条件
    :param Stock ref_stk: 参照证券，通常使用 sh000300 沪深300
    :param int n: 时间窗口
    :param bool spearman: 使用 spearman 相关系数，否则为 pearson)");

    m.def(
      "ICIR",
      [](const Indicator& ind, const py::object& stks, const KQuery& query, const Stock& ref_stk,
         int n, int rolling_n, bool spearman) {
          if (py::isinstance<Block>(stks)) {
              const auto& blk = stks.cast<Block&>();
              return ICIR(ind, blk, query, ref_stk, n, rolling_n, spearman);
          }

          if (py::isinstance<py::sequence>(stks)) {
              StockList c_stks = python_list_to_vector<Stock>(stks);
              return ICIR(ind, c_stks, query, ref_stk, n, rolling_n, spearman);
          }

          HKU_THROW("Input stks must be Block or sequenc(Stock)!");
      },
      py::arg("ind"), py::arg("stks"), py::arg("query"), py::arg("ref_stk"), py::arg("n") = 1,
      py::arg("rolling_n") = 120, py::arg("spearman") = true,
      R"(ICIR(ind, stks, query, ref_stk[, n=1, rolling_n=120])

    计算 IC 因子 IR = IC的多周期均值/IC的标准方差

    :param Indicator ind: 输入因子
    :param sequence(stock)|Block stks 证券组合
    :param Query query: 查询条件
    :param Stock ref_stk: 参照证券，通常使用 sh000300 沪深300
    :param int n: 计算IC时对应的 n 日收益率
    :param int rolling_n: 滚动周期
    :param bool spearman: 使用 spearman 相关系数，否则为 pearson)");

    m.def("ZSCORE", ZSCORE_1, py::arg("out_extreme") = false, py::arg("nsigma") = 3.0,
          py::arg("recursive") = false);
    m.def("ZSCORE", ZSCORE_2, py::arg("data"), py::arg("out_extreme") = false,
          py::arg("nsigma") = 3.0, py::arg("recursive") = false,
          R"(ZSCORE(data[, out_extreme, nsigma, recursive])

    对数据进行标准化（归一），可选进行极值排除

    注：非窗口滚动，如需窗口滚动的标准化，直接 (x - MA(x, n)) / STDEV(x, n) 即可。
    
    :param Indicator data: 待剔除异常值的数据
    :param bool outExtreme: 指示剔除极值，默认 False
    :param float nsigma: 剔除极值时使用的 nsigma 倍 sigma，默认 3.0
    :param bool recursive: 是否进行递归剔除极值，默认 False
    :rtype: Indicator)");

    m.def("TURNOVER", py::overload_cast<int>(TURNOVER), py::arg("n") = 1);
    m.def("TURNOVER", py::overload_cast<const KData&, int>(TURNOVER), py::arg("kdata"),
          py::arg("n") = 1, R"(TURNOVER(data[,n=1])
    换手率=股票成交量/流通股股数×100%

    :param int n: 时间窗口)");

    m.def("RESULT", py::overload_cast<int>(RESULT));
    m.def("RESULT", py::overload_cast<const Indicator&, int>(RESULT), py::arg("data"),
          py::arg("result_ix"), R"(RESULT(data, result_ix)
          
    以公式指标的方式返回指定指标中的指定结果集

    :param Indicator data: 指定的指标
    :param int result_ix: 指定的结果集)");

    m.def("FINANCE", py::overload_cast<int>(FINANCE), py::arg("ix"));
    m.def("FINANCE", py::overload_cast<const string&>(FINANCE), py::arg("name"));
    m.def("FINANCE", py::overload_cast<const KData&, int>(FINANCE), py::arg("kdata"),
          py::arg("ix"));
    m.def("FINANCE", py::overload_cast<const KData&, const string&>(FINANCE), py::arg("kdata"),
          py::arg("name"),
          R"(FINANCE([kdata, ix, name])

    获取历史财务信息。（可通过 StockManager.get_history_finance_all_fields 查询相应的历史财务字段信息）

    ix, name 使用时，为二选一。即要不使用 ix，要不就使用 name 进行获取。

    :param KData kdata: K线数据
    :param int ix: 历史财务信息字段索引
    :param int name: 历史财务信息字段名称)");

    m.def("BLOCKSETNUM", py::overload_cast<const Block&>(BLOCKSETNUM), py::arg("block"));
    m.def("BLOCKSETNUM", py::overload_cast<const Block&, const KQuery&>(BLOCKSETNUM),
          py::arg("block"), py::arg("query"), R"(BLOCKSETNUM(block, query)
    
    横向统计（返回板块股个数）

    :param Block block: 待统计的板块
    :param Query query: 统计范围)");

    m.def(
      "BLOCKSETNUM",
      [](const py::sequence& stks) {
          Block blk;
          blk.add(python_list_to_vector<Stock>(stks));
          return BLOCKSETNUM(blk);
      },
      py::arg("stks"));
    m.def(
      "BLOCKSETNUM",
      [](const py::sequence& stks, const KQuery& query) {
          Block blk;
          blk.add(python_list_to_vector<Stock>(stks));
          return BLOCKSETNUM(blk, query);
      },
      py::arg("stks"), py::arg("query"), R"(BLOCKSETNUM(block, query)
    
    横向统计（返回板块股个数）

    :param Sequence stks: stock list
    :param Query query: 统计范围)");

    m.def("INSUM", py::overload_cast<const Block&, const Indicator&, int, bool>(INSUM),
          py::arg("block"), py::arg("ind"), py::arg("mode"), py::arg("fill_null") = true);
    m.def("INSUM",
          py::overload_cast<const Block&, const KQuery&, const Indicator&, int, bool>(INSUM),
          py::arg("block"), py::arg("query"), py::arg("ind"), py::arg("mode"),
          py::arg("fill_null") = true,
          R"(INSUM(block, query, ind, mode[, fill_null=True])

    返回板块各成分该指标相应输出按计算类型得到的计算值.计算类型:0-累加,1-平均数,2-最大值,3-最小值.

    :param Block block: 指定板块
    :param Query query: 指定范围
    :param Indicator ind: 指定指标
    :param int mode: 计算类型:0-累加,1-平均数,2-最大值,3-最小值.
    :param bool fill_null: 日期对齐时缺失数据填充 nan 值。
    :rtype: Indicator)");

    m.def(
      "INSUM",
      [](const py::sequence stks, const Indicator& ind, int mode, bool fill_null) {
          Block blk;
          blk.add(python_list_to_vector<Stock>(stks));
          return INSUM(blk, ind, mode);
      },
      py::arg("stks"), py::arg("ind"), py::arg("mode"), py::arg("fill_null") = true);
    m.def(
      "INSUM",
      [](const py::sequence stks, const KQuery& query, const Indicator& ind, int mode,
         bool fill_null) {
          Block blk;
          blk.add(python_list_to_vector<Stock>(stks));
          return INSUM(blk, query, ind, mode);
      },
      py::arg("stks"), py::arg("query"), py::arg("ind"), py::arg("mode"),
      py::arg("fill_null") = true,
      R"(INSUM(stks, query, ind, mode[, fill_null=True])

    返回板块各成分该指标相应输出按计算类型得到的计算值.计算类型:0-累加,1-平均数,2-最大值,3-最小值.

    :param Sequence stks: stock list
    :param Query query: 指定范围
    :param Indicator ind: 指定指标
    :param int mode: 计算类型:0-累加,1-平均数,2-最大值,3-最小值.
    :param bool fill_null: 日期对齐时缺失数据填充 nan 值。
    :rtype: Indicator)");

    m.def("ISLASTBAR", py::overload_cast<>(ISLASTBAR));
    m.def("ISLASTBAR", py::overload_cast<const KData&>(ISLASTBAR), py::arg("data"));
    m.def("ISLASTBAR", py::overload_cast<const Indicator&>(ISLASTBAR), py::arg("data"),
          R"(ISLASTBAR(ind)

    判断当前数据是否为最后一个数据，若为最后一个数据，则返回1，否则返回0.

    :param Indicator|KData data: 指定指标
    :rtype: Indicator)");

    m.def("ISNA", py::overload_cast<bool>(ISNA), py::arg("ignore_discard") = false);
    m.def("ISNA", py::overload_cast<const Indicator&, bool>(ISNA), py::arg("ind"),
          py::arg("ignore_discard") = false, R"(ISNA(ind[, ignore_discard=False])

    判断指标是否为 nan 值，若为 nan 值, 则返回1, 否则返回0.

    :param Indicator ind: 指定指标
    :param bool ignore_discard: 忽略指标丢弃数据)");

    m.def("ISINF", py::overload_cast<>(ISINF));
    m.def("ISINF", py::overload_cast<const Indicator&>(ISINF), py::arg("ind"),
          R"(ISINF(ind)

    判断指标是否为正无穷大 (+inf) 值，若为 +inf 值, 则返回1, 否则返回0。如判断负无穷大, 使用 ISINFA。

    :param Indicator ind: 指定指标
    :rtype: Indicator)");

    m.def("ISINFA", py::overload_cast<>(ISINFA));
    m.def("ISINFA", py::overload_cast<const Indicator&>(ISINFA), py::arg("ind"),
          R"(ISINFA(ind)

    判断指标是否为负无穷大 (-inf) 值，若为 -inf 值, 则返回1, 否则返回0。如判断正无穷大, 使用 ISINF。

    :param Indicator ind: 指定指标
    :rtype: Indicator)");
}
