/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MDD test_indicator_MDD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MDD") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, close, mdd;

    /** @arg The normal associated data with n = 10 and a data size of exactly 10 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MDD(c);
    CHECK_EQ(m.name(), "MDD");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,      0.,      0.72282, 0.72282, 0.72282,
                                 3.27389, 3.27389, 3.27389, 3.28155, 3.58515};
    check_indicator(m, PRICELIST(expects));

    /** @arg The normal associated data with n = 0 */
    kdata = stock.getKData(KQuery(-10));
    close = CLOSE(kdata);
    mdd = MDD(close, 0);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);

    check_indicator(mdd, PRICELIST(expects));

    /** @arg n = 5, the normal associated data */
    mdd = MDD(close, 5);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);
    expects = {0., 0., 0.72282, 0.72282, 0.72282, 3.27389, 3.27389, 3.27389, 3.28155, 2.55377};
    check_indicator(mdd, PRICELIST(expects));

    /** @arg n = 1, the normal associated data */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mdd = MDD(close, 1);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);
    Indicator zero_ind = PRICELIST(PriceList(kdata.size(), 0.0));
    check_indicator(mdd, zero_ind);

    /** @arg The empty data test */
    kdata = KData();
    close = CLOSE(kdata);
    mdd = MDD(close, 0);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.size(), 0);
    CHECK_EQ(mdd.empty(), true);

    /** @arg The custom data test */
    PriceList data{100.0, 105.0, 102.0, 108.0, 95.0, 90.0, 101.0, 77.0};
    Indicator d = PRICELIST(data);
    Indicator mdd1 = MDD(d, 5);
    CHECK_EQ(mdd1.discard(), 0);
    expects = {0.0, 0.0, 2.85714, 2.85714, 12.03704, 16.66667, 16.66667, 28.70370};
    check_indicator(mdd1, PRICELIST(expects));

    /** @arg An extreme rising sequence */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mdd_rising = MDD(rising, 0);
    // Use check_indicator to verify that all the values are 0
    zero_ind = PRICELIST(PriceList(rising_data.size(), 0.0));
    check_indicator(mdd_rising, zero_ind);

    /** @arg An extreme falling sequence */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mdd_falling = MDD(falling, 0);
    expects = {0.0, 10.0, 20, 30, 40};
    check_indicator(mdd_falling, PRICELIST(expects));

    mdd_falling = MDD(falling, 3);
    expects = {0.0, 10.0, 20., 22.22222, 25.0};
    check_indicator(mdd_falling, PRICELIST(expects));

    /** @arg The case containing equal prices */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mdd_equal = MDD(equal, 0);
    // Use check_indicator to verify that all the values are 0
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mdd_equal, zero_equal);

    /** @arg A single data point */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mdd_single = MDD(single, 0);
    CHECK_EQ(mdd_single.size(), 1);
    CHECK_EQ(mdd_single[0], 0.0);

    /** @arg Empty data */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mdd_empty = MDD(empty, 0);
    CHECK_EQ(mdd_empty.size(), 0);
    CHECK_EQ(mdd_empty.empty(), true);

    /** @arg The incremental calculation */
    kdata = stock.getKData(KQuery(-20, -10));
    m = MDD(CLOSE(), 3)(kdata);
    mdd1 = m(stock.getKData(-15));
    m = MDD(CLOSE(), 3)(stock.getKData(-15));
    CHECK_EQ(mdd1[0], doctest::Approx(2.4825).epsilon(0.0001));
    CHECK_EQ(mdd1[1], doctest::Approx(2.6372).epsilon(0.0001));
    for (size_t i = 2; i < mdd1.size(); i++) {
        CHECK_EQ(mdd1[i], doctest::Approx(m[i]));
    }

    /** @arg A counter example: a pit is dug first and then a new high is made, pinning the
     * look-ahead bias bug The original incremental algorithm used the global max of the window as
     * the drawdown base; when the highest point of the window appears after the lowest point, it
     * overestimated the drawdown. The data [1.2,1.1,1.0,0.5,1.5,2.0,1.2,1.0], n=5: i=6, the window
     * [1.0,0.5,1.5,2.0,1.2] has its highest 2.0 after the lowest 0.5, the standard MDD=50 (0.5
     * against its preceding accumulated max 1.0) while the buggy version=75 (0.5 against the global
     * max 2.0) i=7, the window [0.5,1.5,2.0,1.2,1.0] likewise, the standard=50 and the buggy
     * version=75 The full calculation with n=5<total=8 goes through the branch B (a longer first
     * segment) + the delegated _increment_calculate, after the fix both segments use the standard
     * run_max base and match the standard MDD per bar.
     */
    PriceList lookahead_data{1.2, 1.1, 1.0, 0.5, 1.5, 2.0, 1.2, 1.0};
    Indicator lookahead = PRICELIST(lookahead_data);
    Indicator mdd_lookahead = MDD(lookahead, 5);
    CHECK_EQ(mdd_lookahead[6], doctest::Approx(50.0).epsilon(0.0001));
    CHECK_EQ(mdd_lookahead[7], doctest::Approx(50.0).epsilon(0.0001));
}

/** @par Test point: the full calculation equals the incremental one (setContext is called
 * repeatedly on the same instance to trigger _increment_calculate) */
TEST_CASE("test_MDD_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    // The full baseline
    Indicator ind_full = MDD(CLOSE(), 5);
    ind_full.setContext(k_full);

    // Incremental: reuse the same instance with consecutive setContext
    Indicator ind_inc = MDD(CLOSE(), 5);
    ind_inc.setContext(k_partial);
    ind_inc.setContext(k_full);

    CHECK_EQ(ind_full.size(), ind_inc.size());
    for (size_t i = 0; i < ind_full.size(); ++i) {
        if (std::isnan(ind_full[i]) && std::isnan(ind_inc[i])) {
            continue;
        }
        CHECK_EQ(ind_inc[i], doctest::Approx(ind_full[i]).epsilon(0.0001));
    }
}

/** @par Test point: the NaN / non-positive data does not crash (no segfault or division by 0) */
TEST_CASE("test_MDD_with_nan") {
    // The data covers the inner and outer defenses of the scheme 4: i=3 (NaN) hits the outer isnan
    // of the incremental path and i=4 (-5) hits the outer <=0, i=5 has a window containing j=3
    // (NaN) hitting the inner isnan and i=6 has a window containing j=4 (-5) hitting the inner <=0
    // n=3<total=7: [0,3) goes through the branch B full first segment and [3,7) delegates to the
    // scheme 4 incremental
    PriceList data{100.0, 105.0, 90.0, std::numeric_limits<double>::quiet_NaN(), -5.0, 90.0, 110.0};
    Indicator mdd = MDD(PRICELIST(data), 3);
    CHECK_EQ(mdd.size(), 7);
    // i=3, the incremental current point is NaN; the scheme 4 continues without writing, keeping
    // the buffer NaN
    CHECK(std::isnan(mdd[3]));
    // i=6, the window [5,6]=[90,110] (skipping the -5 at i=4), run_max=90->110, dd=(90-90)/90=0
    // The actual window [4,6]=[-5,90,110]; after skipping -5 the valid part is [90,110], max=110,
    // dd=0
    CHECK_GE(mdd[6], 0.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_MDD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MDD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MDD(kdata.close());
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    Indicator m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    CHECK_EQ(m2.name(), "MDD");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    // Use check_indicator to verify the consistency before and after the serialization
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
