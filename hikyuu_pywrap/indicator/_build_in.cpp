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
Indicator (*KDATA2)(const Indicator&) = KDATA;
Indicator (*KDATA3)() = KDATA;

Indicator (*OPEN1)(const KData&) = OPEN;
Indicator (*OPEN2)(const Indicator&) = OPEN;
Indicator (*OPEN3)() = OPEN;

Indicator (*HIGH1)(const KData&) = HIGH;
Indicator (*HIGH2)(const Indicator&) = HIGH;
Indicator (*HIGH3)() = HIGH;

Indicator (*LOW1)(const KData&) = LOW;
Indicator (*LOW2)(const Indicator&) = LOW;
Indicator (*LOW3)() = LOW;

Indicator (*CLOSE1)(const KData&) = CLOSE;
Indicator (*CLOSE2)(const Indicator&) = CLOSE;
Indicator (*CLOSE3)() = CLOSE;

Indicator (*AMO1)(const KData&) = AMO;
Indicator (*AMO2)(const Indicator&) = AMO;
Indicator (*AMO3)() = AMO;

Indicator (*VOL1)(const KData&) = VOL;
Indicator (*VOL2)(const Indicator&) = VOL;
Indicator (*VOL3)() = VOL;

Indicator (*KDATA_PART1)(const KData& kdata, const string& part) = KDATA_PART;
Indicator (*KDATA_PART2)(const Indicator& kdata, const string& part) = KDATA_PART;
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
Indicator (*VIGOR_3)(const Indicator&, int) = VIGOR;

Indicator (*WEAVE_1)() = WEAVE;
Indicator (*WEAVE_2)(const Indicator&) = WEAVE;

Indicator (*CVAL_1)(double, size_t, size_t) = CVAL;
Indicator (*CVAL_2)(const Indicator&, double) = CVAL;

Indicator (*IND_AND1)(const Indicator&, const Indicator&) = IND_AND;
Indicator (*IND_AND2)(const Indicator&, price_t) = IND_AND;
Indicator (*IND_AND3)(price_t, const Indicator&) = IND_AND;

Indicator (*IND_OR1)(const Indicator&, const Indicator&) = IND_OR;
Indicator (*IND_OR2)(const Indicator&, price_t) = IND_OR;
Indicator (*IND_OR3)(price_t, const Indicator&) = IND_OR;

void export_Indicator_build_in() {
    docstring_options doc_options(false);

    def("KDATA", KDATA1);
    def("KDATA", KDATA2);
    def("KDATA", KDATA3);

    def("CLOSE", CLOSE1);
    def("CLOSE", CLOSE2);
    def("CLOSE", CLOSE3);

    def("OPEN", OPEN1);
    def("OPEN", OPEN2);
    def("OPEN", OPEN3);

    def("HIGH", HIGH1);
    def("HIGH", HIGH2);
    def("HIGH", HIGH3);

    def("LOW", LOW1);
    def("LOW", LOW2);
    def("LOW", LOW3);

    def("AMO", AMO1);
    def("AMO", AMO2);
    def("AMO", AMO3);

    def("VOL", VOL1);
    def("VOL", VOL2);
    def("VOL", VOL3);

    def("KDATA_PART", KDATA_PART1, (arg("data"), arg("kpart")));
    def("KDATA_PART", KDATA_PART2, (arg("data"), arg("kpart")));
    def("KDATA_PART", KDATA_PART3, (arg("kpart")));

    //无法直接使用Null<size_t>()，会导致异常退出
    size_t null_size = Null<size_t>();
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
    def("VIGOR", VIGOR_3, (arg("ind"), arg("n")=2));

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

    def("WEAVE", WEAVE_1);
    def("WEAVE", WEAVE_2);

    def("CVAL", CVAL_1, (arg("value")=0.0, arg("len")=0, arg("discard")=0));
    def("CVAL", CVAL_2, (arg("data"), arg("value")=0.0));

    def("IND_AND", IND_AND1);
    def("IND_AND", IND_AND2);
    def("IND_AND", IND_AND3);

    def("IND_OR", IND_OR1);
    def("IND_OR", IND_OR2);
    def("IND_OR", IND_OR3);

}


