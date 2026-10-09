/*
 * test_DROPNA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DROPNA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DROPNA test_indicator_DROPNA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_DROPNA") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg There is no nan value */
    result = DROPNA(data);
    CHECK_EQ(result.name(), "DROPNA");
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg All the values are nan */
    a.clear();
    for (int i = 0; i < 10; i++) {
        a.push_back(Null<price_t>());
    }

    data = VALUE(a);
    result = DROPNA(data);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg There is a nan value in the middle */
    a.push_back(Null<price_t>());
    a.push_back(12);
    a.push_back(Null<price_t>());
    a.push_back(15);
    a.push_back(Null<price_t>());
    data = VALUE(a);
    result = DROPNA(data);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 12);
    CHECK_EQ(result[1], 15);
}

/** @par Test points */
TEST_CASE("test_DROPNA_multi_result") {
    const double nan = Null<price_t>();

    PriceList c, d, e;
    for (size_t i = 0; i < 10; ++i) {
        c.push_back(double(i));
        d.push_back(double(i * 10));
        e.push_back(double(i * 100));
    }
    c[2] = nan;  // one row invalid in the first result set
    d[5] = nan;  // another row invalid in the second result set
    e[5] = nan;  // the same row, in the third result set

    Indicator two = WEAVE(PRICELIST(c), PRICELIST(d));
    Indicator three = WEAVE(PRICELIST(c), PRICELIST(d), PRICELIST(e));
    CHECK_EQ(two.getResultNumber(), 2);
    CHECK_EQ(three.getResultNumber(), 3);

    /** @arg every result set keeps its own values, one row per kept input row */
    Indicator r2 = DROPNA(two);
    CHECK_EQ(r2.getResultNumber(), 2);
    CHECK_EQ(r2.size(), 8);
    CHECK_EQ(r2.discard(), 0);
    PriceList keep = {0.0, 1.0, 3.0, 4.0, 6.0, 7.0, 8.0, 9.0};
    for (size_t i = 0; i < 8; ++i) {
        CHECK_EQ(r2.get(i, 0), doctest::Approx(keep[i]));
        CHECK_EQ(r2.get(i, 1), doctest::Approx(keep[i] * 10));
    }

    /** @arg three result sets are gathered the same way */
    Indicator r3 = DROPNA(three);
    CHECK_EQ(r3.getResultNumber(), 3);
    CHECK_EQ(r3.size(), 8);
    for (size_t i = 0; i < 8; ++i) {
        CHECK_EQ(r3.get(i, 0), doctest::Approx(keep[i]));
        CHECK_EQ(r3.get(i, 1), doctest::Approx(keep[i] * 10));
        CHECK_EQ(r3.get(i, 2), doctest::Approx(keep[i] * 100));
    }

    /** @arg the kept dates match the kept rows */
    CHECK_EQ(r3.getDatetimeList().size(), r3.size());
}

/** @par Test points */
TEST_CASE("test_DROPNA_multi_result_boundary") {
    const double nan = Null<price_t>();
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));

    PriceList c, d;
    for (size_t i = 0; i < 10; ++i) {
        c.push_back(double(i));
        d.push_back(nan);
    }

    /** @arg every result set nan keeps no row */
    Indicator allnan = DROPNA(WEAVE(PRICELIST(c), PRICELIST(d)));
    CHECK_EQ(allnan.size(), 0);
    CHECK_EQ(allnan.discard(), 0);
    CHECK_EQ(allnan.getResultNumber(), 2);

    /** @arg the input discard is honored as the gather start */
    Indicator r = DROPNA(WEAVE(MA(CLOSE(), 3), CLOSE()));
    r.setContext(k_full);
    CHECK_EQ(r.getResultNumber(), 2);
    CHECK_EQ(r.size(), 18);
    CHECK_EQ(r.discard(), 0);
    Indicator ma = MA(CLOSE(), 3);
    ma.setContext(k_full);
    for (size_t i = 0; i < 18; ++i) {
        CHECK_EQ(r.get(i, 0), doctest::Approx(ma[i + 2]));
        CHECK_EQ(r.get(i, 1), doctest::Approx(k_full[i + 2].closePrice));
    }

    /** @arg the layout holds for the maximum result set count */
    PriceList l0, l1, l2, l3, l4, l5;
    for (size_t i = 0; i < 10; ++i) {
        l0.push_back(double(i));
        l1.push_back(double(i + 1));
        l2.push_back(double(i + 2));
        l3.push_back(double(i + 3));
        l4.push_back(double(i + 4));
        l5.push_back(double(i + 5));
    }
    l0[2] = nan;  // dropped in the first result set
    l4[5] = nan;  // dropped in the fifth result set
    Indicator six = WEAVE(PRICELIST(l0), PRICELIST(l1), PRICELIST(l2), PRICELIST(l3), PRICELIST(l4),
                          PRICELIST(l5));
    CHECK_EQ(six.getResultNumber(), 6);
    Indicator r6 = DROPNA(six);
    CHECK_EQ(r6.getResultNumber(), 6);
    CHECK_EQ(r6.size(), 8);
    PriceList kept = {0.0, 1.0, 3.0, 4.0, 6.0, 7.0, 8.0, 9.0};
    for (size_t i = 0; i < 8; ++i) {
        for (size_t col = 0; col < 6; ++col) {
            CHECK_EQ(r6.get(i, col), doctest::Approx(double(col) + kept[i]));
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_DROPNA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DROPNA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = DROPNA(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "DROPNA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
