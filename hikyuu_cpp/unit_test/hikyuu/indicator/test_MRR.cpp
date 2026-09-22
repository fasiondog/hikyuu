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
#include <hikyuu/indicator/crt/MRR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MRR test_indicator_MRR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MRR") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, close, mrr;

    /** @arg The normal associated data with n = 10 and a data size of exactly 10 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MRR(c);
    CHECK_EQ(m.name(), "MRR");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,      0.103922, 0.103922, 0.118056, 1.35151,
                                 1.35151, 2.29046,  2.29046,  2.29046,  2.29046};
    check_indicator(m, PRICELIST(expects));

    /** @arg The normal associated data with n = 0 */
    kdata = stock.getKData(KQuery(-10));
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    check_indicator(mrr, PRICELIST(expects));

    /** @arg n = 1, the normal associated data */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mrr = MRR(close, 1);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    Indicator zero_ind = PRICELIST(PriceList(kdata.size(), 0.0));
    check_indicator(mrr, zero_ind);

    /** @arg The empty data test */
    kdata = KData();
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.size(), 0);
    CHECK_EQ(mrr.empty(), true);

    /** @arg An extreme rising sequence */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mrr_rising = MRR(rising, 0);
    expects = {0.0, 10.0, 20.0, 30.0, 40.0};
    check_indicator(mrr_rising, PRICELIST(expects));

    mrr_rising = MRR(rising, 3);
    expects = {0.0, 10.0, 20.0, 18.181818, 16.66667};
    check_indicator(mrr_rising, PRICELIST(expects));

    /** @arg An extreme falling sequence */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mrr_falling = MRR(falling, 0);
    Indicator zero_ind2 = PRICELIST(PriceList(falling_data.size(), 0.0));
    check_indicator(mrr_falling, zero_ind2);

    /** @arg The case containing equal prices */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mrr_equal = MRR(equal, 0);
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mrr_equal, zero_equal);

    /** @arg A single data point */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mrr_single = MRR(single, 0);
    CHECK_EQ(mrr_single.size(), 1);
    CHECK_EQ(mrr_single[0], 0.0);

    /** @arg Empty data */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mrr_empty = MRR(empty, 0);
    CHECK_EQ(mrr_empty.size(), 0);
    CHECK_EQ(mrr_empty.empty(), true);

    /** @arg The incremental calculation */
    kdata = stock.getKData(KQuery(-20, -10));
    m = MRR(CLOSE(), 3)(kdata);
    mrr = m(stock.getKData(-15));
    m = MRR(CLOSE(), 3)(stock.getKData(-15));
    for (size_t i = 10; i < mrr.size(); i++) {
        CHECK_EQ(mrr[i], doctest::Approx(m[i]));
    }

    /** @arg A counter example: a high is made first and then a pit is dug, pinning the look-ahead
     * bias bug (symmetric to IMdd) The original incremental algorithm used the global min of the
     * window as the rise base; when the lowest point of the window appears after the highest point,
     * it overestimated the rise rate. The data [1.0, 2.0, 1.5, 0.5, 1.2], n=4:
     *   i=4, the window [2.0, 1.5, 0.5, 1.2] has its lowest 0.5 after the highest 2.0,
     *     the standard MRR=140 (1.2 against its preceding accumulated min 0.5) while the buggy
     * version=300 (2.0 against the global min 0.5) The full calculation with n=4<total=5 goes
     * through the branch B (a longer first segment) + the delegated _increment_calculate, after the
     * fix both segments use the standard run_min base and match the standard MRR.
     */
    PriceList mrr_lookahead_data{1.0, 2.0, 1.5, 0.5, 1.2};
    Indicator mrr_lookahead = MRR(PRICELIST(mrr_lookahead_data), 4);
    CHECK_EQ(mrr_lookahead[4], doctest::Approx(140.0).epsilon(0.0001));
}

/** @par Test point: the full calculation equals the incremental one (setContext is called
 * repeatedly on the same instance to trigger _increment_calculate) */
TEST_CASE("test_MRR_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    Indicator ind_full = MRR(CLOSE(), 5);
    ind_full.setContext(k_full);

    Indicator ind_inc = MRR(CLOSE(), 5);
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
TEST_CASE("test_MRR_with_nan") {
    // The data covers the inner and outer defenses of the scheme 4: i=3 (NaN) hits the outer isnan
    // of the incremental path and i=4 (-5) hits the outer <=0, i=5 has a window containing j=3
    // (NaN) hitting the inner isnan and i=6 has a window containing j=4 (-5) hitting the inner <=0
    // n=3<total=7: [0,3) goes through the branch B full first segment and [3,7) delegates to the
    // scheme 4 incremental
    PriceList data{1.0, 2.0, 1.5, std::numeric_limits<double>::quiet_NaN(), -5.0, 0.5, 1.2};
    Indicator mrr = MRR(PRICELIST(data), 3);
    CHECK_EQ(mrr.size(), 7);
    // i=3, the incremental current point is NaN; the scheme 4 continues without writing, keeping
    // the buffer NaN
    CHECK(std::isnan(mrr[3]));
    // i=6, the window [4,6]=[-5,0.5,1.2]; skipping -5 the valid part is [0.5,1.2], run_min=0.5,
    // rr=1.2/0.5-1=140
    CHECK_GE(mrr[6], 0.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_MRR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MRR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MRR(kdata.close());
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

    CHECK_EQ(m2.name(), "MRR");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    // Use check_indicator to verify the consistency before and after the serialization
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */