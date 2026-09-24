/*
 * test_SUMBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-7
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/SUMBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_SUMBARS test_indicator_SUMBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SUMBARS") {
    Indicator result;

    PriceList a;

    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);
    }
    Indicator data = PRICELIST(a);

    result = SUMBARS(data, 10);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 15);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 90);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 1);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 170);
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result.size(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[6], 3);
    CHECK_EQ(result[7], 2);
    CHECK_EQ(result[8], 2);
    CHECK_EQ(result[9], 1);

    result = SUMBARS(data, 450);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 9);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[9], 8);

    result = SUMBARS(data, 451);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = SUMBARS(CVAL(10), 451);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    result = SUMBARS(CVAL(10), 10);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    a.clear();
    a.push_back(10);
    a.push_back(20);
    data = PRICELIST(a);
    result = SUMBARS(data, 20);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 30);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 1);

    result = SUMBARS(data, 40);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
}

/** @par Test points */
TEST_CASE("test_SUMBARS_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = SUMBARS(c, 10);
    Indicator result = SUMBARS(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = SUMBARS(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = SUMBARS(c, 0);
    result = SUMBARS(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @par Test points */
TEST_CASE("test_SUMBARS_with_cval_dyn_param") {
    // The original trigger scenario: a nested CVAL used as the SUMBARS dynamic parameter.
    // Before the fix: one=CVAL(10) inside CVAL(one, 10) -> goes through Indicator::operator() ->
    // alike true
    //   -> it short-circuits and returns an uncalculated empty shell (size==0) -> SUMBARS throws
    //   the HKU_CHECK(ind_param.size()==ind.size()) exception.
    // After the fix: ind is reused, CVAL(one,10).size()==1 and SUMBARS calculates normally.

    Indicator one = CVAL(10);
    CHECK_EQ(one.size(), 1);
    CHECK_EQ(one[0], 10);

    Indicator seq = CVAL(one, 10);
    CHECK_EQ(seq.size(), one.size());  // 0 before the fix
    CHECK_EQ(seq[0], 10);

    // Reachable: ind[0]=10 >= a[0]=10, satisfied at the first bar, the distance is 0
    Indicator r = SUMBARS(one, seq);
    CHECK_EQ(r.size(), 1);
    CHECK_EQ(r[0], 0);

    // Unreachable: 10 < 20
    Indicator seq2 = CVAL(one, 20);
    CHECK_EQ(seq2.size(), one.size());
    Indicator r2 = SUMBARS(one, seq2);
    CHECK_EQ(r2.size(), 1);
    CHECK_UNARY(std::isnan(r2[0]));
}

//-----------------------------------------------------------------------------
// The sign regression of the dynamic/static paths (fixing the negative sign bug of _dyn_calculate)
//-----------------------------------------------------------------------------
/** @par Test points */
TEST_CASE("test_SUMBARS_dyn_sign_regression") {
    // A counter example: the last bar must go back 2 bars to accumulate to a, the distance is > 0
    // (a non-degenerate solution)
    //   before the fix the dynamic version returned -2 (a wrong sign) and after the fix it returns
    //   2 (matching the static one)
    PriceList a;
    a.push_back(5);
    a.push_back(3);
    a.push_back(2);  // [5, 3, 2]
    Indicator data = PRICELIST(a);

    // The static path (a scalar parameter)
    Indicator s = SUMBARS(data, 10);
    CHECK_UNARY(std::isnan(s[0]));
    CHECK_UNARY(std::isnan(s[1]));
    CHECK_EQ(s[2], 2);  // i=2 goes back to j=0, the distance is 2

    // The dynamic path (an IndParam sequence parameter, a constant sequence)
    Indicator d1 = SUMBARS(data, CVAL(data, 10));
    CHECK_UNARY(std::isnan(d1[0]));
    CHECK_UNARY(std::isnan(d1[1]));
    CHECK_EQ(d1[2], 2);  // -2 before the fix

    Indicator d2 = SUMBARS(data, IndParam(CVAL(data, 10)));
    CHECK_EQ(d2[2], 2);

    // The per-bar value symmetry of the dynamic/static paths (the discard need not be equal, see
    // the case below)
    CHECK_EQ(s.size(), d1.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (std::isnan(s[i])) {
            CHECK_UNARY(std::isnan(d1[i]));
        } else {
            CHECK_EQ(s[i], doctest::Approx(d1[i]));
        }
    }
}

/** @par Test points */
TEST_CASE("test_SUMBARS_dyn_static_equivalence") {
    // Several positions have a distance > 0, covering the non-degenerate solution equivalence class
    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);  // [0,10,20,30,40,50,60,70,80,90]
    }
    Indicator data = PRICELIST(a);

    // Static: discard=4, [4]=2, [5..8]=1, [9]=0
    Indicator s = SUMBARS(data, 90);
    CHECK_EQ(s[4], 2);
    CHECK_EQ(s[5], 1);
    CHECK_EQ(s[6], 1);
    CHECK_EQ(s[7], 1);
    CHECK_EQ(s[8], 1);
    CHECK_EQ(s[9], 0);

    auto check_eq = [](const Indicator& x, const Indicator& y) {
        CHECK_EQ(x.size(), y.size());
        for (size_t i = 0; i < x.size(); i++) {
            if (std::isnan(x[i])) {
                CHECK_UNARY(std::isnan(y[i]));
            } else {
                CHECK_EQ(x[i], doctest::Approx(y[i]));
            }
        }
    };

    check_eq(s, SUMBARS(data, CVAL(data, 90)));
    check_eq(s, SUMBARS(data, IndParam(CVAL(data, 90))));
}

/** @par Test points */
TEST_CASE("test_SUMBARS_dyn_unreachable_discard") {
    // The whole-range accumulation is unreachable: the dynamic version writes NaN per bar without
    // advancing the discard, while the static version discards everything
    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(1);  // All 1
    }
    Indicator data = PRICELIST(a);

    Indicator s = SUMBARS(data, 100);  // The whole range is unreachable
    CHECK_EQ(s.discard(), s.size());   // The static version discards everything
    for (size_t i = 0; i < s.size(); i++) {
        CHECK_UNARY(std::isnan(s[i]));
    }

    Indicator d = SUMBARS(data, CVAL(data, 100));
    CHECK_EQ(d.discard(), data.discard());  // The dynamic version keeps the input discard
    for (size_t i = 0; i < d.size(); i++) {
        CHECK_UNARY(std::isnan(d[i]));
    }
}

/** @par Test points */
TEST_CASE("test_SUMBARS_dyn_edge") {
    // 1. a <= 0: sum>=a holds at the first bar so the distance is always 0 (verifying the i==j
    // degenerate solution, no infinite loop)
    {
        PriceList a;
        for (int i = 0; i < 5; i++)
            a.push_back(i + 1);  // [1,2,3,4,5]
        Indicator data = PRICELIST(a);

        Indicator s0 = SUMBARS(data, 0);
        Indicator d0 = SUMBARS(data, CVAL(data, 0));
        for (size_t i = 0; i < data.size(); i++) {
            CHECK_EQ(s0[i], 0);
            CHECK_EQ(d0[i], 0);
        }

        Indicator sneg = SUMBARS(data, -5);
        Indicator dneg = SUMBARS(data, CVAL(data, -5));
        for (size_t i = 0; i < data.size(); i++) {
            CHECK_EQ(sneg[i], 0);
            CHECK_EQ(dneg[i], 0);
        }
    }

    // 2. The input sequence has a discard > 0: the lower bound of the inner j loop is
    // ind.discard(), verifying the termination without an out of range access
    {
        PriceList a;
        a.push_back(10);
        for (int i = 0; i < 6; i++)
            a.push_back(1);                  // [10,1,1,1,1,1,1]
        Indicator ma = MA(PRICELIST(a), 2);  // discard=1, [1]=(10+1)/2=5.5, and then always 1
        CHECK_EQ(ma.discard(), 1);

        Indicator s = SUMBARS(ma, 2);
        Indicator d = SUMBARS(ma, CVAL(ma, 2));
        CHECK_EQ(d.discard(), ma.discard());
        CHECK_EQ(s[1], 0);  // j=1 sum=5.5>=2
        CHECK_EQ(d[1], 0);
        CHECK_EQ(s[2], 1);  // j=2 sum=1; j=1 sum=6.5>=2 -> i-j=1
        CHECK_EQ(d[2], 1);
        CHECK_EQ(s[3], 1);
        CHECK_EQ(d[3], 1);
    }

    // 3. The extreme lengths
    {
        // An empty sequence: no crash
        PriceList empty;
        Indicator data = PRICELIST(empty);
        Indicator s = SUMBARS(data, 10);
        Indicator d = SUMBARS(data, CVAL(data, 10));
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(d.size(), 0);

        // A single reachable bar (the static path can use CVAL; the dynamic path builds an a
        // sequence of size=1 with PRICELIST, avoiding the defect that a nested CVAL propagates a
        // size of 0 on a single element)
        Indicator one = CVAL(10);
        CHECK_EQ(SUMBARS(one, 10)[0], 0);
        PriceList one_pl;
        one_pl.push_back(10);
        CHECK_EQ(SUMBARS(one, IndParam(PRICELIST(one_pl)))[0], 0);

        // A single unreachable bar
        CHECK_UNARY(std::isnan(SUMBARS(one, 20)[0]));
        PriceList unreach_pl;
        unreach_pl.push_back(20);
        CHECK_UNARY(std::isnan(SUMBARS(one, IndParam(PRICELIST(unreach_pl)))[0]));
    }
}

/** @par Test points */
TEST_CASE("test_SUMBARS_dyn_varying_param") {
    // The dynamic parameter a sequence varies per bar, verified by hand
    PriceList a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(2);
    a.push_back(1);  // data=[1,2,3,2,1]
    Indicator data = PRICELIST(a);

    PriceList pa;
    pa.push_back(5);
    pa.push_back(100);
    pa.push_back(3);
    pa.push_back(3);
    pa.push_back(3);  // a=[5,100,3,3,3]
    Indicator aseq = PRICELIST(pa);

    Indicator d = SUMBARS(data, IndParam(aseq));
    CHECK_UNARY(std::isnan(d[0]));  // a=5, accumulated to j=0 sum=1 < 5
    CHECK_UNARY(std::isnan(d[1]));  // a=100, unreachable
    CHECK_EQ(d[2], 0);              // a=3, j=2 sum=3>=3
    CHECK_EQ(d[3], 1);              // a=3, j=3 sum=2, j=2 sum=5>=3 -> i-j=1
    CHECK_EQ(d[4], 1);              // a=3, j=4 sum=1, j=3 sum=3>=3 -> i-j=1
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SUMBARS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SUMBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SUMBARS(CLOSE(kdata), 10000);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x2.name(), "SUMBARS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
