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

Indicator (*DATE1)() = DATE;
Indicator (*DATE2)(const KData&) = DATE;

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
Indicator (*IF_2)(const Indicator&, price_t, const Indicator&) = IF;
Indicator (*IF_3)(const Indicator&, const Indicator&, price_t) = IF;
Indicator (*IF_4)(const Indicator&, price_t, price_t) = IF;

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
Indicator (*POW_6)(price_t, int) = POW;

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
Indicator (*CROSS_2)(const Indicator&, price_t) = CROSS;
Indicator (*CROSS_3)(price_t, const Indicator&) = CROSS;
Indicator (*CROSS_4)(price_t, price_t) = CROSS;

Indicator (*LONGCROSS_1)(const Indicator&, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_2)(const Indicator&, const Indicator&, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_3)(const Indicator&, price_t, int) = LONGCROSS;
Indicator (*LONGCROSS_4)(const Indicator&, price_t, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_5)(price_t, const Indicator&, int) = LONGCROSS;
Indicator (*LONGCROSS_6)(price_t, const Indicator&, const Indicator&) = LONGCROSS;
Indicator (*LONGCROSS_7)(price_t, price_t, int) = LONGCROSS;
Indicator (*LONGCROSS_8)(price_t, price_t, const Indicator&) = LONGCROSS;

Indicator (*FILTER_1)(int) = FILTER;
Indicator (*FILTER_2)(const IndParam&) = FILTER;
Indicator (*FILTER_3)(const Indicator&, const IndParam&) = FILTER;
Indicator (*FILTER_4)(const Indicator&, const Indicator&) = FILTER;
Indicator (*FILTER_5)(const Indicator&, int) = FILTER;

Indicator (*BARSSINCE_1)() = BARSSINCE;
Indicator (*BARSSINCE_2)(const Indicator&) = BARSSINCE;
Indicator (*BARSSINCE_3)(price_t) = BARSSINCE;

Indicator (*BARSLAST_1)() = BARSLAST;
Indicator (*BARSLAST_2)(const Indicator&) = BARSLAST;
Indicator (*BARSLAST_3)(price_t) = BARSLAST;

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

Indicator (*ALIGN_1)(const DatetimeList&) = ALIGN;
Indicator (*ALIGN_2)(const Indicator&, const DatetimeList&) = ALIGN;
Indicator (*ALIGN_3)(const Indicator&, const Indicator&) = ALIGN;
Indicator (*ALIGN_4)(const Indicator&, const KData&) = ALIGN;

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

void export_Indicator_build_in() {
    def("KDATA", KDATA1);
    def("KDATA", KDATA3, R"(KDATA([data])

    包装KData成Indicator，用于其他指标计算

    :param data: KData 或 具有6个返回结果的Indicator（如KDATA生成的Indicator）
    :rtype: Indicator)");

    def("CLOSE", CLOSE1);
    def("CLOSE", CLOSE3, R"(CLOSE([data])

    获取收盘价，包装KData的收盘价成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    def("OPEN", OPEN1);
    def("OPEN", OPEN3, R"(OPEN([data])

    获取开盘价，包装KData的开盘价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    def("HIGH", HIGH1);
    def("HIGH", HIGH3, R"(HIGH([data])

    获取最高价，包装KData的最高价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    def("LOW", LOW1);
    def("LOW", LOW3, R"(LOW([data])

    获取最低价，包装KData的最低价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator)");

    def("AMO", AMO1);
    def("AMO", AMO3, R"(AMO([data])

    获取成交金额，包装KData的成交金额成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    def("VOL", VOL1);
    def("VOL", VOL3, R"(VOL([data])

    获取成交量，包装KData的成交量成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator)");

    def("KDATA_PART", KDATA_PART1, (arg("data"), arg("kpart")));
    def("KDATA_PART", KDATA_PART3, (arg("kpart")), R"(KDATA_PART([data, kpart])

    根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL，如:KDATA_PART("CLOSE")等同于CLOSE()

    :param data: 输入数据（KData 或 Indicator） 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :rtype: Indicator)");

    def("DATE", DATE1);
    def("DATE", DATE2, R"(DATE([data])

    取得该周期从1900以来的年月日。用法: DATE 例如函数返回1000101，表示2000年1月1日。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("TIME", TIME1);
    def("TIME", TIME2, R"(TIME([data])

    取得该周期的时分秒。用法: TIME 函数返回有效值范围为(000000-235959)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("YEAR", YEAR1);
    def("YEAR", YEAR2, R"(YEAR([data])

    取得该周期的年份。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("MONTH", MONTH1);
    def("MONTH", MONTH2, R"(MONTH([data])

    取得该周期的月份。用法: MONTH 函数返回有效值范围为(1-12)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("WEEK", WEEK1);
    def("WEEK", WEEK2, R"(WEEK([data])

    取得该周期的星期数。用法：WEEK 函数返回有效值范围为(0-6)，0表示星期天。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("DAY", DAY1);
    def("DAY", DAY2, R"(DAY([data])

    取得该周期的日期。用法: DAY 函数返回有效值范围为(1-31)。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("HOUR", HOUR1);
    def("HOUR", HOUR2, R"(HOUR([data])

    取得该周期的小时数。用法：HOUR 函数返回有效值范围为(0-23)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("MINUTE", MINUTE1);
    def("MINUTE", MINUTE2, R"(MINUTE([data])

    取得该周期的分钟数。用法：MINUTE 函数返回有效值范围为(0-59)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator)");

    def("PRICELIST", PRICELIST2, (arg("data"), arg("discard") = 0));
    def("PRICELIST", PRICELIST3, (arg("data"), arg("result_index") = 0));
    def("PRICELIST", PRICELIST4, (arg("result_index") = 0));

    def("SMA", SMA_1, (arg("n") = 22, arg("m") = 2.0));
    def("SMA", SMA_2, (arg("n"), arg("m")));
    def("SMA", SMA_3, (arg("n"), arg("m") = 2.0));
    def("SMA", SMA_4, (arg("n"), arg("m")));
    def("SMA", SMA_5, (arg("data"), arg("n") = 22, arg("m") = 2.0));
    def("SMA", SMA_6, (arg("data"), arg("n"), arg("m")));
    def("SMA", SMA_7, (arg("data"), arg("n"), arg("m") = 2.0));
    def("SMA", SMA_8, (arg("data"), arg("n"), arg("m")));
    def("SMA", SMA_9, (arg("data"), arg("n"), arg("m")));
    def("SMA", SMA_10, (arg("data"), arg("n"), arg("m") = 2.0));
    def("SMA", SMA_11, (arg("data"), arg("n"), arg("m")), R"(SMA([data, n=22, m=2])

    求移动平均

    用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :param float|Indicator|IndParam m: 系数
    :rtype: Indicator)");

    def("EMA", EMA_1, (arg("n") = 22));
    def("EMA", EMA_2, (arg("n")));
    def("EMA", EMA_3, (arg("data"), arg("n")));
    def("EMA", EMA_4, (arg("data"), arg("n")));
    def("EMA", EMA_5, (arg("data"), arg("n") = 22), R"(EMA([data, n=22])

    指数移动平均线(Exponential Moving Average)

    :param data: 输入数据
    :param int|Indicator|IndParam n n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator)");

    def("MA", MA_1, (arg("n") = 22));
    def("MA", MA_2, (arg("n")));
    def("MA", MA_3, (arg("data"), arg("n")));
    def("MA", MA_4, (arg("data"), arg("n")));
    def("MA", MA_5, (arg("data"), arg("n") = 22), R"(MA([data, n=22])

    简单移动平均

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("AMA", AMA_1, (arg("n") = 10, arg("fast_n") = 2, arg("slow_n") = 30));
    def("AMA", AMA_2, (arg("n"), arg("fast_n"), arg("slow_n")));
    def("AMA", AMA_4, (arg("n"), arg("fast_n"), arg("slow_n")));
    def("AMA", AMA_5, (arg("n"), arg("fast_n"), arg("slow_n")));
    def("AMA", AMA_3, (arg("data"), arg("n") = 10, arg("fast_n") = 2, arg("slow_n") = 30),
        R"(AMA([data, n=10, fast_n=2, slow_n=30])

    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于2的整数
    :param int|Indicator|IndParam fast_n: 对应快速周期N
    :param int|Indicator|IndParam slow_n: 对应慢速EMA线的N值
    :rtype: Indicator

    * result(0): AMA
    * result(1): ER)");

    def("ATR", ATR_1, (arg("n") = 14));
    def("ATR", ATR_2, (arg("n")));
    def("ATR", ATR_3, (arg("data"), arg("n")));
    def("ATR", ATR_4, (arg("data"), arg("n")));
    def("ATR", ATR_5, (arg("data"), arg("n") = 14), R"(ATR([data, n=14])

    平均真实波幅(Average True Range)

    :param Indicator data 待计算的源数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于1的整数
    :rtype: Indicator)");

    def("MACD", MACD_1, (arg("n1") = 12, arg("n2") = 26, arg("n3") = 9));
    def("MACD", MACD_2, (arg("n1"), arg("n2"), arg("n3")));
    def("MACD", MACD_3, (arg("data"), arg("n1") = 12, arg("n2") = 26, arg("n3") = 9));
    def("MACD", MACD_4, (arg("data"), arg("n1"), arg("n2"), arg("n3")));
    def("MACD", MACD_5, (arg("data"), arg("n1"), arg("n2"), arg("n3")),
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

    def("VIGOR", VIGOR_1, (arg("kdata"), arg("n") = 2));
    def("VIGOR", VIGOR_2, (arg("n") = 2), R"(VIGOR([kdata, n=2])

    亚历山大.艾尔德力度指数 [BOOK2]_

    计算公式：（收盘价今－收盘价昨）＊成交量今

    :param KData data: 输入数据
    :param int n: EMA平滑窗口
    :rtype: Indicator)");

    def("SAFTYLOSS", SAFTYLOSS_1, (arg("n1") = 10, arg("n2") = 3, arg("p") = 2.0));
    def("SAFTYLOSS", SAFTYLOSS_2, (arg("n1"), arg("n2"), arg("p") = 2.0));
    def("SAFTYLOSS", SAFTYLOSS_3, (arg("n1"), arg("n2"), arg("p")));
    def("SAFTYLOSS", SAFTYLOSS_4, (arg("data"), arg("n1") = 10, arg("n2") = 3, arg("p") = 2.0));
    def("SAFTYLOSS", SAFTYLOSS_5, (arg("data"), arg("n1"), arg("n2"), arg("p") = 2.0));
    def("SAFTYLOSS", SAFTYLOSS_6, (arg("data"), arg("n1"), arg("n2"), arg("p")));
    def("SAFTYLOSS", SAFTYLOSS_7, (arg("data"), arg("n1"), arg("n2"), arg("p") = 2.0));
    def("SAFTYLOSS", SAFTYLOSS_8, (arg("data"), arg("n1"), arg("n2"), arg("p")),
        R"(SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_

    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n1: 计算平均噪音的回溯时间窗口
    :param int|Indicator|IndParam n2: 对初步止损线去n2日内的最高值
    :param float|Indicator|IndParam p: 噪音系数
    :rtype: Indicator)");

    def("DIFF", DIFF_1);
    def("DIFF", DIFF_2, R"(DIFF([data])

    差分指标，即data[i] - data[i-1]

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("REF", REF_1, (arg("n")));
    def("REF", REF_2, (arg("n")));
    def("REF", REF_3, (arg("data"), arg("n")));
    def("REF", REF_4, (arg("data"), arg("n")));
    def("REF", REF_5, (arg("data"), arg("n")), R"(REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。

    用法：REF(X，A)　引用A周期前的X值。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 引用n周期前的值，即右移n位
    :rtype: Indicator)");

    def("STDEV", STDEV_1, (arg("n") = 10));
    def("STDEV", STDEV_2, (arg("n")));
    def("STDEV", STDEV_3, (arg("data"), arg("n")));
    def("STDEV", STDEV_4, (arg("data"), arg("n")));
    def("STDEV", STDEV_5, (arg("data"), arg("n") = 10), R"(STDEV([data, n=10])

    计算N周期内样本标准差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("STDP", STDP_1, (arg("n") = 10));
    def("STDP", STDP_2, (arg("n")));
    def("STDP", STDP_3, (arg("data"), arg("n")));
    def("STDP", STDP_4, (arg("data"), arg("n")));
    def("STDP", STDP_5, (arg("data"), arg("n") = 10), R"(STDP([data, n=10])

    总体标准差，STDP(X,N)为X的N日总体标准差

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    def("POS", POS, (arg("block"), arg("query"), arg("sg")));

    def("HHV", HHV_1, (arg("n") = 20));
    def("HHV", HHV_2, (arg("n")));
    def("HHV", HHV_3, (arg("data"), arg("n")));
    def("HHV", HHV_4, (arg("data"), arg("n")));
    def("HHV", HHV_5, (arg("data"), arg("n") = 20), R"(HHV([data, n=20])

    N日内最高价，N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    def("LLV", LLV_1, (arg("n") = 20));
    def("LLV", LLV_2, (arg("n")));
    def("LLV", LLV_3, (arg("data"), arg("n")));
    def("LLV", LLV_4, (arg("data"), arg("n")));
    def("LLV", LLV_5, (arg("data"), arg("n") = 20), R"(LLV([data, n=20])

    N日内最低价，N=0则从第一个有效值开始。

    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    def("CVAL", CVAL_1, (arg("value") = 0.0, arg("discard") = 0));
    def("CVAL", CVAL_2, (arg("data"), arg("value") = 0.0, arg("discard") = 0),
        R"(CVAL([data, value=0.0, discard=0])

    data 为 Indicator 实例，创建和 data 等长的常量指标，其值和为value，抛弃长度discard和data一样

    :param Indicator data: Indicator实例
    :param float value: 常数值
    :param int discard: 抛弃数量
    :rtype: Indicator)");

    def("LIUTONGPAN", LIUTONGPAN_1);
    def("LIUTONGPAN", LIUTONGPAN_2, R"(LIUTONGPAN(kdata)

   获取流通盘（单位：万股），同 CAPITAL

   :param KData kdata: k线数据
   :rtype: Indicator)");

    def("HSL", HSL_1);
    def("HSL", HSL_2, R"(HSL(kdata)

    获取换手率，等于 VOL(k) / CAPITAL(k)

    :param KData kdata: k线数据
    :rtype: Indicator)");

    def("WEAVE", WEAVE, R"(WEAVE(ind1, ind2)

    将ind1和ind2的结果组合在一起放在一个Indicator中。如ind = WEAVE(ind1, ind2), 则此时ind包含多个结果，按ind1、ind2的顺序存放。
    
    :param Indicator ind1: 指标1
    :param Indicator ind2: 指标2
    :rtype: Indicator)");

    def("CORR", CORR, R"(CORR(ind1, ind2, n)

    计算 ind1 和 ind2 的相关系数。返回中存在两个结果，第一个为相关系数，第二个为协方差。

    :param Indicator ind1: 指标1
    :param Indicator ind2: 指标2
    :param int n: 按指定 n 的长度计算两个 ind 直接数据相关系数
    :rtype: Indicator)");

    def("IF", IF_1);
    def("IF", IF_2);
    def("IF", IF_3);
    def("IF", IF_4, R"(IF(x, a, b)

    条件函数, 根据条件求不同的值。

    用法：IF(X,A,B)若X不为0则返回A,否则返回B

    例如：IF(CLOSE>OPEN,HIGH,LOW)表示该周期收阳则返回最高值,否则返回最低值

    :param Indicator x: 条件指标
    :param Indicator a: 待选指标 a
    :param Indicator b: 待选指标 b
    :rtype: Indicator)");

    def("COUNT", COUNT_1, (arg("n") = 20));
    def("COUNT", COUNT_2, (arg("n")));
    def("COUNT", COUNT_3, (arg("data"), arg("n")));
    def("COUNT", COUNT_4, (arg("data"), arg("n")));
    def("COUNT", COUNT_5, (arg("data"), arg("n") = 20), R"(COUNT([data, n=20])

    统计满足条件的周期数。

    用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。

    例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数

    :param Indicator data: 条件
    :param int|Indicator|IndParam n: 周期
    :rtype: Indicator)");

    def("SUM", SUM_1, (arg("n") = 20));
    def("SUM", SUM_2, (arg("n")));
    def("SUM", SUM_3, (arg("data"), arg("n")));
    def("SUM", SUM_4, (arg("data"), arg("n")));
    def("SUM", SUM_5, (arg("data"), arg("n") = 20), R"(SUM([data, n=20])

    求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("ABS", ABS_1);
    def("ABS", ABS_2);
    def("ABS", ABS_3, R"(ABS([data])

    求绝对值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("NOT", NOT_1);
    def("NOT", NOT_2, R"(NOT([data])

    求逻辑非。NOT(X)返回非X,即当X=0时返回1，否则返回0。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("SGN", SGN_1);
    def("SGN", SGN_2);
    def("SGN", SGN_3, R"(SGN([data])

    求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("EXP", EXP_1);
    def("EXP", EXP_2);
    def("EXP", EXP_3, R"(EXP([data])

    EXP(X)为e的X次幂

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("MAX", MAX_1);
    def("MAX", MAX_2);
    def("MAX", MAX_3, R"(MAX(ind1, ind2)

    求最大值, MAX(A,B)返回A和B中的较大值。

    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator)");

    def("MIN", MIN_1);
    def("MIN", MIN_2);
    def("MIN", MIN_3, R"(MIN(ind1, ind2)

    求最小值, MIN(A,B)返回A和B中的较小值。

    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator)");

    def("BETWEEN", BETWEEN_1);
    def("BETWEEN", BETWEEN_2);
    def("BETWEEN", BETWEEN_3);
    def("BETWEEN", BETWEEN_4);
    def("BETWEEN", BETWEEN_5);
    def("BETWEEN", BETWEEN_6);
    def("BETWEEN", BETWEEN_7);
    def("BETWEEN", BETWEEN_8, R"(BETWEEN(a, b, c)

    介于(介于两个数之间)

    用法：BETWEEN(A,B,C)表示A处于B和C之间时返回1，否则返回0

    例如：BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5))表示收盘价介于5日均线和10日均线之间

    :param Indicator a: A
    :param Indicator b: B
    :param Indicator c: C
    :rtype: Indicator)");

    def("LN", LN_1);
    def("LN", LN_2);
    def("LN", LN_3, R"(LN([data])

    求自然对数, LN(X)以e为底的对数

    :param data: 输入数据
    :rtype: Indicator)");

    def("LOG", LOG_1);
    def("LOG", LOG_2);
    def("LOG", LOG_3, R"(LOG([data])

    以10为底的对数

    :param data: 输入数据
    :rtype: Indicator)");

    def("HHVBARS", HHVBARS_1, (arg("n") = 20));
    def("HHVBARS", HHVBARS_2, (arg("n")));
    def("HHVBARS", HHVBARS_3, (arg("data"), arg("n")));
    def("HHVBARS", HHVBARS_4, (arg("data"), arg("n")));
    def("HHVBARS", HHVBARS_5, (arg("data"), arg("n") = 20), R"(HHVBARS([data, n=20])

    上一高点位置 求上一高点到当前的周期数。

    用法：HHVBARS(X,N):求N周期内X最高值到当前周期数N=0表示从第一个有效值开始统计

    例如：HHVBARS(HIGH,0)求得历史新高到到当前的周期数

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    def("LLVBARS", LLVBARS_1, (arg("n") = 20));
    def("LLVBARS", LLVBARS_2, (arg("n")));
    def("LLVBARS", LLVBARS_3, (arg("data"), arg("n")));
    def("LLVBARS", LLVBARS_4, (arg("data"), arg("n")));
    def("LLVBARS", LLVBARS_5, (arg("data"), arg("n") = 20), R"(LLVBARS([data, n=20])

    上一低点位置 求上一低点到当前的周期数。

    用法：LLVBARS(X,N):求N周期内X最低值到当前周期数N=0表示从第一个有效值开始统计

    例如：LLVBARS(HIGH,20)求得20日最低点到当前的周期数

    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator)");

    def("POW", POW_1, (arg("n")));
    def("POW", POW_2, (arg("n")));
    def("POW", POW_3, (arg("data"), arg("n")));
    def("POW", POW_4, (arg("data"), arg("n")));
    def("POW", POW_5, (arg("data"), arg("n")));
    def("POW", POW_6), (arg("data"), arg("n"), R"(POW(data, n)

    乘幂

    用法：POW(A,B)返回A的B次幂

    例如：POW(CLOSE,3)求得收盘价的3次方

    :param data: 输入数据
    :param int|Indicator|IndParam n: 幂
    :rtype: Indicator)");

    def("SQRT", SQRT_1);
    def("SQRT", SQRT_2);
    def("SQRT", SQRT_3, R"(SQRT([data])

    开平方

    用法：SQRT(X)为X的平方根

    例如：SQRT(CLOSE)收盘价的平方根

    :param data: 输入数据
    :rtype: Indicator)");

    def("ROUND", ROUND_1, (arg("ndigits") = 2));
    def("ROUND", ROUND_2, (arg("data"), arg("ndigits") = 2));
    def("ROUND", ROUND_3, (arg("data"), arg("ndigits") = 2), R"(ROUND([data, ndigits=2])

    四舍五入

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    def("ROUNDUP", ROUNDUP_1, (arg("ndigits") = 2));
    def("ROUNDUP", ROUNDUP_2, (arg("data"), arg("ndigits") = 2));
    def("ROUNDUP", ROUNDUP_3, (arg("data"), arg("ndigits") = 2), R"(ROUNDUP([data, ndigits=2])

    向上截取，如10.1截取后为11

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    def("ROUNDDOWN", ROUNDDOWN_1, (arg("ndigits") = 2));
    def("ROUNDDOWN", ROUNDDOWN_2, (arg("data"), arg("ndigits") = 2));
    def("ROUNDDOWN", ROUNDDOWN_3, (arg("data"), arg("ndigits") = 2), R"(ROUND([data, ndigits=2])

    四舍五入

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator)");

    def("FLOOR", FLOOR_1);
    def("FLOOR", FLOOR_2);
    def("FLOOR", FLOOR_3, R"(FLOOR([data])

    向下舍入(向数值减小方向舍入)取整

    用法：FLOOR(A)返回沿A数值减小方向最接近的整数

    例如：FLOOR(12.3)求得12

    :param data: 输入数据
    :rtype: Indicator)");

    def("CEILING", CEILING_1);
    def("CEILING", CEILING_2);
    def("CEILING", CEILING_3, R"(CEILING([data])

    向上舍入(向数值增大方向舍入)取整
   
    用法：CEILING(A)返回沿A数值增大方向最接近的整数
   
    例如：CEILING(12.3)求得13；CEILING(-3.5)求得-3
   
    :param data: 输入数据
    :rtype: Indicator)");

    def("INTPART", INTPART_1);
    def("INTPART", INTPART_2);
    def("INTPART", INTPART_3, R"(INTPART([data])

    取整(绝对值减小取整，即取得数据的整数部分)

    :param data: 输入数据
    :rtype: Indicator)");

    def("EXIST", EXIST_1, (arg("n") = 20));
    def("EXIST", EXIST_2, (arg("n")));
    def("EXIST", EXIST_3, (arg("data"), arg("n")));
    def("EXIST", EXIST_4, (arg("data"), arg("n")));
    def("EXIST", EXIST_5, (arg("data"), arg("n") = 20), R"(EXIST([data, n=20])

    存在, EXIST(X,N) 表示条件X在N周期有存在

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator)");

    def("EVERY", EVERY_1, (arg("n") = 20));
    def("EVERY", EVERY_2, (arg("n")));
    def("EVERY", EVERY_3, (arg("data"), arg("n")));
    def("EVERY", EVERY_4, (arg("data"), arg("n")));
    def("EVERY", EVERY_5, (arg("data"), arg("n") = 20), R"(EVERY([data, n=20])

    一直存在

    用法：EVERY (X,N) 表示条件X在N周期一直存在

    例如：EVERY(CLOSE>OPEN,10) 表示前10日内一直是阳线

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator)");

    def("LAST", LAST_1, (arg("m") = 10, arg("n") = 5));
    def("LAST", LAST_2, (arg("m"), arg("n")));
    def("LAST", LAST_3, (arg("m"), arg("n") = 5));
    def("LAST", LAST_4, (arg("m"), arg("n")));
    def("LAST", LAST_5, (arg("data"), arg("m") = 10, arg("n") = 5));
    def("LAST", LAST_6, (arg("data"), arg("m"), arg("n")));
    def("LAST", LAST_7, (arg("data"), arg("m"), arg("n") = 5));
    def("LAST", LAST_8, (arg("data"), arg("m"), arg("n")));
    def("LAST", LAST_9, (arg("data"), arg("m"), arg("n")));
    def("LAST", LAST_10, (arg("data"), arg("m"), arg("n") = 5));
    def("LAST", LAST_11, (arg("data"), arg("m"), arg("n")), R"(LAST([data, m=10, n=5])

    区间存在。

    用法：LAST (X,M,N) 表示条件 X 在前 M 周期到前 N 周期存在。

    例如：LAST(CLOSE>OPEN,10,5) 表示从前10日到前5日内一直阳线。

    :param data: 输入数据
    :param int|Indicator|IndParam m: m周期
    :param int|Indicator|IndParam n: n周期
    :rtype: Indicator)");

    def("SIN", SIN_1);
    def("SIN", SIN_2);
    def("SIN", SIN_3, R"(SIN([data])

    正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("ASIN", ASIN_1);
    def("ASIN", ASIN_2);
    def("ASIN", ASIN_3, R"(ASIN([data])

    反正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("COS", COS_1);
    def("COS", COS_2);
    def("COS", COS_3, R"(COS([data])

    余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("ACOS", ACOS_1);
    def("ACOS", ACOS_2);
    def("ACOS", ACOS_3, R"(ACOS([data])

    反余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("TAN", TAN_1);
    def("TAN", TAN_2);
    def("TAN", TAN_3, R"(TAN([data])

    正切值

    :param Indicator data: 输入数据
    :rtype: Indicato)");

    def("ATAN", ATAN_1);
    def("ATAN", ATAN_2);
    def("ATAN", ATAN_3, R"(ATAN([data])

    反正切值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("REVERSE", REVERSE_1);
    def("REVERSE", REVERSE_2);
    def("REVERSE", REVERSE_3, R"(REVERSE([data])

    求相反数，REVERSE(X)返回-X

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("MOD", MOD_1);
    def("MOD", MOD_2);
    def("MOD", MOD_3);
    def("MOD", MOD_4, R"(MOD(ind1, ind2)

    取整后求模。该函数仅为兼容通达信。实际上，指标求模可直接使用 % 操作符

    用法：MOD(A,B)返回A对B求模

    例如：MOD(26,10) 返回 6

    :param Indicator ind1:
    :param Indicator ind2:
    :rtype: Indicator)");

    def("VAR", VAR_1, (arg("n") = 10));
    def("VAR", VAR_2, (arg("n")));
    def("VAR", VAR_3, (arg("data"), arg("n")));
    def("VAR", VAR_4, (arg("data"), arg("n")));
    def("VAR", VAR_5, (arg("data"), arg("n") = 10), R"(VAR([data, n=10])

    估算样本方差, VAR(X,N)为X的N日估算样本方差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("VARP", VARP_1, (arg("n") = 10));
    def("VARP", VARP_2, (arg("n")));
    def("VARP", VARP_3, (arg("data"), arg("n")));
    def("VARP", VARP_4, (arg("data"), arg("n")));
    def("VARP", VARP_5, (arg("data"), arg("n") = 10), R"(VARP([data, n=10])

    总体样本方差, VARP(X,N)为X的N日总体样本方差

    :param Indicator data: 输入数据
    :param int n|Indicator|IndParam: 时间窗口
    :rtype: Indicator)");

    def("UPNDAY", UPNDAY_1, (arg("data"), arg("n") = 3));
    def("UPNDAY", UPNDAY_2, (arg("data"), arg("n")));
    def("UPNDAY", UPNDAY_3, (arg("data"), arg("n")), R"(UPNDAY(data[, n=3])

    连涨周期数, UPNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("DOWNNDAY", DOWNNDAY_1, (arg("data"), arg("n") = 3));
    def("DOWNNDAY", DOWNNDAY_2, (arg("data"), arg("n")));
    def("DOWNNDAY", DOWNNDAY_3, (arg("data"), arg("n")), R"(DOWNNDAY(data[, n=3])

    连跌周期数, DOWNNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("NDAY", NDAY_1, (arg("x"), arg("y"), arg("n") = 3));
    def("NDAY", NDAY_1, (arg("x"), arg("y"), arg("n")));
    def("NDAY", NDAY_3, (arg("x"), arg("y"), arg("n")), R"(NDAY(x, y[, n=3])

    连大, NDAY(X,Y,N)表示条件X>Y持续存在N个周期

    :param Indicator x:
    :param Indicator y:
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("CROSS", CROSS_1);
    def("CROSS", CROSS_2);
    def("CROSS", CROSS_3);
    def("CROSS", CROSS_4, R"(CROSS(x, y)

    交叉函数

    :param x: 变量或常量，判断交叉的第一条线
    :param y: 变量或常量，判断交叉的第二条线
    :rtype: Indicator)");

    def("LONGCROSS", LONGCROSS_1, (arg("a"), arg("b"), arg("n") = 3));
    def("LONGCROSS", LONGCROSS_2, (arg("a"), arg("b"), arg("n")));
    def("LONGCROSS", LONGCROSS_3, (arg("a"), arg("b"), arg("n") = 3));
    def("LONGCROSS", LONGCROSS_4, (arg("a"), arg("b"), arg("n")));
    def("LONGCROSS", LONGCROSS_5, (arg("a"), arg("b"), arg("n") = 3));
    def("LONGCROSS", LONGCROSS_6, (arg("a"), arg("b"), arg("n")));
    def("LONGCROSS", LONGCROSS_7, (arg("a"), arg("b"), arg("n") = 3));
    def("LONGCROSS", LONGCROSS_8, (arg("a"), arg("b"), arg("n")), R"(LONGCROSS(a, b[, n=3])

    两条线维持一定周期后交叉

    用法：LONGCROSS(A,B,N)表示A在N周期内都小于B，本周期从下方向上穿过B时返 回1，否则返回0

    例如：LONGCROSS(MA(CLOSE,5),MA(CLOSE,10),5)表示5日均线维持5周期后与10日均线交金叉

    :param Indicator a:
    :param Indicator b:
    :param int|Indicator n:
    :rtype: Indicator)");

    def("FILTER", FILTER_1, (arg("n") = 5));
    def("FILTER", FILTER_2, (arg("n")));
    def("FILTER", FILTER_3, (arg("data"), arg("n")));
    def("FILTER", FILTER_4, (arg("data"), arg("n")));
    def("FILTER", FILTER_5, (arg("data"), arg("n") = 5), R"(FILTER([data, n=5])

    信号过滤, 过滤连续出现的信号。

    用法：FILTER(X,N): X 满足条件后，删除其后 N 周期内的数据置为 0。

    例如：FILTER(CLOSE>OPEN,5) 查找阳线，5 天内再次出现的阳线不被记录在内。

    :param Indicator data: 输入数据
    :param int|Indicaot|IndParam n: 过滤周期
    :rtype: Indicator)");

    def("BARSSINCE", BARSSINCE_1);
    def("BARSSINCE", BARSSINCE_2);
    def("BARSSINCE", BARSSINCE_3, R"(BARSSINCE([data])

    第一个条件成立位置到当前的周期数。

    用法：BARSSINCE(X):第一次X不为0到现在的天数。

    例如：BARSSINCE(HIGH>10)表示股价超过10元时到当前的周期数

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("BARSLAST", BARSLAST_1);
    def("BARSLAST", BARSLAST_2);
    def("BARSLAST", BARSLAST_3, R"(BARSLAST([data])

    上一次条件成立位置 上一次条件成立到当前的周期数。

    用法：BARSLAST(X): 上一次 X 不为 0 到现在的天数。

    例如：BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) 表示上一个涨停板到当前的周期数

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("SUMBARS", SUMBARS_1, (arg("a")));
    def("SUMBARS", SUMBARS_2, (arg("a")));
    def("SUMBARS", SUMBARS_3, (arg("data"), arg("a")));
    def("SUMBARS", SUMBARS_4, (arg("data"), arg("a")));
    def("SUMBARS", SUMBARS_5, (arg("data"), arg("a")), R"(SUMBARS([data,] a)

    累加到指定周期数, 向前累加到指定值到现在的周期数

    用法：SUMBARS(X,A):将X向前累加直到大于等于A,返回这个区间的周期数

    例如：SUMBARS(VOL,CAPITAL)求完全换手到现在的周期数

    :param Indicator data: 输入数据
    :param float a|Indicator|IndParam: 指定累加和
    :rtype: Indicator)");

    def("BARSCOUNT", BARSCOUNT_1);
    def("BARSCOUNT", BARSCOUNT_2, R"(BARSCOUNT([data])

    有效值周期数, 求总的周期数。

    用法：BARSCOUNT(X)第一个有效数据到当前的天数。

    例如：BARSCOUNT(CLOSE)对于日线数据取得上市以来总交易日数，对于1分钟线取得当日交易分钟数。

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("BACKSET", BACKSET_1, (arg("n") = 2));
    def("BACKSET", BACKSET_2, (arg("n")));
    def("BACKSET", BACKSET_3, (arg("data"), arg("n")));
    def("BACKSET", BACKSET_4, (arg("data"), arg("n")));
    def("BACKSET", BACKSET_5, (arg("data"), arg("n") = 2), R"(BACKSET([data, n=2])

    向前赋值将当前位置到若干周期前的数据设为1。

    用法：BACKSET(X,N),X非0,则将当前位置到N周期前的数值设为1。

    例如：BACKSET(CLOSE>OPEN,2)若收阳则将该周期及前一周期数值设为1,否则为0

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N周期
    :rtype: Indicator)");

    def("TIMELINE", TIMELINE_1);
    def("TIMELINE", TIMELINE_2, R"(TIMELINE([k])

    分时价格数据

    :param KData k: 上下文
    :rtype: Indicator)");

    def("TIMELINEVOL", TIMELINEVOL_1);
    def("TIMELINEVOL", TIMELINEVOL_2, R"(TIMELINEVOL([k])

    分时成交量数据

    :param KData k: 上下文
    :rtype: Indicator)");

    def("DMA", DMA, R"(DMA(ind, a)

    动态移动平均

    用法：DMA(X,A),求X的动态移动平均。

    算法：若Y=DMA(X,A) 则 Y=A*X+(1-A)*Y',其中Y'表示上一周期Y值。

    例如：DMA(CLOSE,VOL/CAPITAL)表示求以换手率作平滑因子的平均价

    :param Indicator ind: 输入数据
    :param Indicator a: 动态系数
    :rtype: Indicator)");

    def("AVEDEV", AVEDEV_1, (arg("data"), arg("n") = 22));
    def("AVEDEV", AVEDEV_2, (arg("data"), arg("n")));
    def("AVEDEV", AVEDEV_3, (arg("data"), arg("n")), R"(AVEDEV(data[, n=22])

    平均绝对偏差，求X的N日平均绝对偏差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("DEVSQ", DEVSQ_1, (arg("n") = 10));
    def("DEVSQ", DEVSQ_2, (arg("n")));
    def("DEVSQ", DEVSQ_3, (arg("data"), arg("n")));
    def("DEVSQ", DEVSQ_4, (arg("data"), arg("n")));
    def("DEVSQ", DEVSQ_5, (arg("data"), arg("n") = 10), R"(DEVSQ([data, n=10])

    数据偏差平方和，求X的N日数据偏差平方和

    :param Indicator data: 输入数据
    :param int|Indicator n: 时间窗口
    :rtype: Indicator)");

    def("ROC", ROC_1, (arg("n") = 10));
    def("ROC", ROC_2, (arg("n")));
    def("ROC", ROC_3, (arg("data"), arg("n")));
    def("ROC", ROC_4, (arg("data"), arg("n")));
    def("ROC", ROC_5, (arg("data"), arg("n") = 10), R"(ROC([data, n=10])

    变动率指标: ((price / prevPrice)-1)*100

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    def("ROCP", ROCP_1, (arg("n") = 10));
    def("ROCP", ROCP_2, (arg("n")));
    def("ROCP", ROCP_3, (arg("data"), arg("n")));
    def("ROCP", ROCP_4, (arg("data"), arg("n")));
    def("ROCP", ROCP_5, (arg("data"), arg("n") = 10), R"(ROCP([data, n=10])

    变动率指标: (price - prevPrice) / prevPrice

    :param data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator)");

    def("ROCR", ROCR_1, (arg("n") = 10));
    def("ROCR", ROCR_2, (arg("n")));
    def("ROCR", ROCR_3, (arg("data"), arg("n")));
    def("ROCR", ROCR_4, (arg("data"), arg("n")));
    def("ROCR", ROCR_5, (arg("data"), arg("n") = 10), R"(ROCR([data, n=10])

    变动率指标: (price / prevPrice)

    :param data: 输入数据
    :param int n|Indicator|IndParam: 时间窗口
    :rtype: Indicator)");

    def("ROCR100", ROCR100_1, (arg("n") = 10));
    def("ROCR100", ROCR100_2, (arg("n")));
    def("ROCR100", ROCR100_3, (arg("data"), arg("n")));
    def("ROCR100", ROCR100_4, (arg("data"), arg("n")));
    def("ROCR100", ROCR100_5, (arg("data"), arg("n") = 10), R"(ROCR100([data, n=10])

    变动率指标: (price / prevPrice) * 100

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("AD", AD_1);
    def("AD", AD_2, R"(AD(kdata)

   累积/派发线

   :param KData kdata: k线数据
   :rtype: Indicator)");

    def("COST", COST_1, (arg("x") = 10.0));
    def("COST", COST_2, (arg("k"), arg("x") = 10.0), R"(COST(k[, x=10.0])

    成本分布

    用法：COST(k, X) 表示X%获利盘的价格是多少

    例如：COST(k, 10),表示10%获利盘的价格是多少，即有10%的持仓量在该价格以下，其余90%在该价格以上，为套牢盘 该函数仅对日线分析周期有效

    :param KData k: 关联的K线数据
    :param float x: x%获利价格, 0~100
    :rtype: Indicator)");

    def("ALIGN", ALIGN_1);
    def("ALIGN", ALIGN_2);
    def("ALIGN", ALIGN_3);
    def("ALIGN", ALIGN_4, R"(ALIGN(data, ref):

    按指定的参考日期对齐

    :param Indicator data: 输入数据
    :param ref: 指定做为日期参考的 DatetimeList、Indicator 或 KData
    :retype: Indicator)");

    def("DROPNA", DROPNA_1);
    def("DROPNA", DROPNA_2, R"(DROPNA([data])

    删除 nan 值

    :param Indicator data: 输入数据
    :rtype: Indicator)");

    def("ADVANCE", ADVANCE,
        (arg("query") = KQueryByIndex(-100), arg("market") = "SH", arg("stk_type") = STOCKTYPE_A,
         arg("ignore_context") = false),
        R"(ADVANCE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    上涨家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :rtype: Indicator)");

    def("DECLINE", DECLINE,
        (arg("query") = KQueryByIndex(-100), arg("market") = "SH", arg("stk_type") = STOCKTYPE_A,
         arg("ignore_context") = false),
        R"(DECLINE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    下跌家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :rtype: Indicator)");

    def("SLICE", SLICE_1, (arg("data"), arg("start"), arg("end")));
    def("SLICE", SLICE_2, (arg("start"), arg("end"), arg("result_index") = 0));
    def("SLICE", SLICE_3, (arg("data"), arg("start"), arg("end"), arg("result_index") = 0),
        R"(SLICE(data, start, end, result_index=0)

    获取某指标中指定范围 [start, end) 的数据，生成新的指标

    :param Indicator|PriceList data: 输入数据
    :param int start: 起始位置
    :param int end: 终止位置（不包含本身）
    :param int result_index: 原输入数据中的结果集)");

    def("RSI", RSI_1, (arg("n") = 14));
    def("RSI", RSI_2, (arg("data"), arg("n") = 14), R"(RSI([data, n=14])

    相对强弱指数

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");

    def("SLOPE", SLOPE1, (arg("n") = 22));
    def("SLOPE", SLOPE2, (arg("n")));
    def("SLOPE", SLOPE3, (arg("data"), arg("n") = 22));
    def("SLOPE", SLOPE4, (arg("data"), arg("n")));
    def("SLOPE", SLOPE5, (arg("data"), arg("n")), R"(SLOPE([data, n=22])

    计算线性回归斜率，N支持变量

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator)");
}
