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

Indicator (*PRICELIST1)(const PriceList&) = PRICELIST;
Indicator (*PRICELIST2)(const PriceList&, size_t, size_t) = PRICELIST;
Indicator (*PRICELIST3)(const Indicator&, size_t, size_t, size_t) = PRICELIST;

Indicator (*AMA_1)(int, int, int) = AMA;
Indicator (*AMA_2)(const Indicator&, int, int, int) = AMA;
BOOST_PYTHON_FUNCTION_OVERLOADS(AMA_1_overload, AMA, 0, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(AMA_2_overload, AMA, 1, 4);

Indicator (*DIFF_1)() = DIFF;
Indicator (*DIFF_2)(const Indicator&) = DIFF;

Indicator (*MA_1)(int) = MA;
Indicator (*MA_2)(const Indicator&, int) = MA;

Indicator (*EMA_1)(int) = EMA;
Indicator (*EMA_2)(const Indicator&, int) = EMA;

Indicator (*MACD_1)(int, int, int) = MACD;
Indicator (*MACD_2)(const Indicator&, int, int, int) = MACD;
BOOST_PYTHON_FUNCTION_OVERLOADS(MACD_1_overload, MACD, 0, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(MACD_2_overload, MACD, 1, 4);

Indicator (*SAFTYLOSS_1)(int n1, int n2, double p) = SAFTYLOSS;
Indicator (*SAFTYLOSS_2)(const Indicator&, int n1, int n2, double p) = SAFTYLOSS;
BOOST_PYTHON_FUNCTION_OVERLOADS(SAFTYLOSS_1_overload, SAFTYLOSS, 0, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(SAFTYLOSS_2_overload, SAFTYLOSS, 1, 4);

Indicator (*STDEV_1)(int) = STDEV;
Indicator (*STDEV_2)(const Indicator&, int) = STDEV;

void export_Indicator_build_in() {
    def("KDATA", KDATA);
    def("CLOSE", CLOSE);
    def("OPEN", OPEN);
    def("HIGH", HIGH);
    def("LOW", LOW);
    def("TRANSAMOUNT", TRANSAMOUNT);
    def("TRANSCOUNT", TRANSCOUNT);
    def("AMO", AMO);
    def("VOL", VOL);
    def("KDATA_PART", KDATA_PART, (arg("kdata"), arg("part")),
            "part = 'KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL|TRANSAMOUNT|TRANSCOUNT'");

    def("PRICELIST", PRICELIST1);
    def("PRICELIST", PRICELIST2);
    def("PRICELIST", PRICELIST3);

    def("MA", MA_1);
    def("MA", MA_2);
    def("EMA", EMA_1);
    def("EMA", EMA_2);
    def("AMA", AMA_1, AMA_1_overload());
    def("AMA", AMA_2, AMA_2_overload());
    def("MACD", MACD_1, MACD_1_overload());
    def("MACD", MACD_2, MACD_2_overload());
    def("VIGOR", VIGOR);
    def("SAFTYLOSS", SAFTYLOSS_1, SAFTYLOSS_1_overload());
    def("SAFTYLOSS", SAFTYLOSS_2, SAFTYLOSS_2_overload());
    def("DIFF", DIFF_1);
    def("DIFF", DIFF_2);
    def("STDEV", STDEV_1);
    def("STDEV", STDEV_2);
}


