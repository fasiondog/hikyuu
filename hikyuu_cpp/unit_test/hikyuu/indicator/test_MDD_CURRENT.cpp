/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD_CURRENT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MDD_CURRENT test_indicator_MDD_CURRENT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MDD_CURRENT") {
    /** @arg The basic functionality test */
    PriceList data{100.0, 95.0, 90.0, 98.0, 105.0, 102.0};
    Indicator d = PRICELIST(data);
    Indicator mdd_current = MDD_CURRENT(d);
    CHECK_EQ(mdd_current.name(), "MDD_CURRENT");
    CHECK_EQ(mdd_current.empty(), false);
    CHECK_EQ(mdd_current.size(), 6);
    CHECK_EQ(mdd_current.discard(), 0);

    // The calculation verification:
    // 100 -> the historical high=100 -> the drawdown=0%
    // 95 -> the historical high=100 -> the drawdown=5%
    // 90 -> the historical high=100 -> the drawdown=10%
    // 98 -> the historical high=100 -> the drawdown=2%
    // 105 -> the historical high=105 -> the drawdown=0%
    // 102 -> the historical high=105 -> the drawdown~2.86%
    std::vector<price_t> expects{0.0, 5.0, 10.0, 2.0, 0.0, 2.857142857142857};
    check_indicator(mdd_current, PRICELIST(expects));

    /** @arg An extreme rising sequence */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mdd_rising = MDD_CURRENT(rising);
    Indicator zero_ind = PRICELIST(PriceList(rising_data.size(), 0.0));
    check_indicator(mdd_rising, zero_ind);

    /** @arg An extreme falling sequence */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mdd_falling = MDD_CURRENT(falling);
    expects = {0.0, 10.0, 20.0, 30.0, 40.0};
    check_indicator(mdd_falling, PRICELIST(expects));

    /** @arg The case containing equal prices */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mdd_equal = MDD_CURRENT(equal);
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mdd_equal, zero_equal);

    /** @arg A single data point */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mdd_single = MDD_CURRENT(single);
    CHECK_EQ(mdd_single.size(), 1);
    CHECK_EQ(mdd_single[0], 0.0);

    /** @arg Empty data */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mdd_empty = MDD_CURRENT(empty);
    CHECK_EQ(mdd_empty.size(), 0);
    CHECK_EQ(mdd_empty.empty(), true);

    /** @arg The fluctuating sequence test */
    PriceList volatile_data{50.0, 60.0, 55.0, 70.0, 65.0, 80.0, 75.0, 85.0, 80.0, 90.0};
    Indicator volatile_ind = PRICELIST(volatile_data);
    Indicator mdd_volatile = MDD_CURRENT(volatile_ind);
    // The calculation verification:
    // 50 → 50 → 0%
    // 60 → 60 → 0%
    // 55 → 60 → 8.33%
    // 70 → 70 → 0%
    // 65 → 70 → 7.14%
    // 80 → 80 → 0%
    // 75 → 80 → 6.25%
    // 85 → 85 → 0%
    // 80 → 85 → 5.88%
    // 90 → 90 → 0%
    expects = {0.0,  0.0, 8.333333333333334, 0.0, 7.142857142857143, 0.0,
               6.25, 0.0, 5.882352941176471, 0.0};
    check_indicator(mdd_volatile, PRICELIST(expects));
}

/** @par Test point: the handling of the NaN / negative data */
TEST_CASE("test_MDD_CURRENT_with_nan") {
    PriceList data{100.0, 105.0, 90.0, std::numeric_limits<double>::quiet_NaN(), -5.0, 90.0, 110.0};
    Indicator mdd = MDD_CURRENT(PRICELIST(data));
    CHECK_EQ(mdd.size(), 7);
    CHECK_EQ(mdd[3], 0.0);                                        // NaN is filled with 0
    CHECK_EQ(mdd[4], doctest::Approx(104.7619).epsilon(0.0001));  // -5, high 105 -> 104.7619%
    CHECK_EQ(mdd[5], doctest::Approx(14.2857).epsilon(0.0001));   // 90, high 105 -> 14.2857%
    CHECK_EQ(mdd[6], 0.0);                                        // 110, a new historical high
}

/** @par Test point: the incremental calculation */
TEST_CASE("test_MDD_CURRENT_increment") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    // Verify the incremental calculation with the indicator chaining
    Indicator m = MDD_CURRENT(CLOSE())(kdata);
    CHECK_EQ(m.name(), "MDD_CURRENT");
    CHECK_EQ(m.size(), kdata.size());

    // Verify the incremental calculation with consecutive calls
    KData kdata_more = stock.getKData(KQuery(-25));
    Indicator m_more = m(kdata_more);
    CHECK_EQ(m_more.size(), kdata_more.size());

    // Compare with the direct calculation
    Indicator m_direct = MDD_CURRENT(CLOSE())(kdata_more);
    CHECK_EQ(m_more.size(), m_direct.size());
    for (size_t i = 0; i < m_more.size(); ++i) {
        if (std::isnan(m_more[i]) && std::isnan(m_direct[i])) {
            continue;
        }
        CHECK_EQ(m_more[i], doctest::Approx(m_direct[i]).epsilon(0.0001));
    }
}

/** @par Test point: the full calculation equals the incremental one (setContext is called
 * repeatedly on the same instance to trigger _increment_calculate) */
TEST_CASE("test_MDD_CURRENT_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    // The full baseline
    Indicator ind_full = MDD_CURRENT(CLOSE());
    ind_full.setContext(k_full);

    // Incremental: reuse the same instance with consecutive setContext
    Indicator ind_inc = MDD_CURRENT(CLOSE());
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

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_MDD_CURRENT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MDD_CURRENT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MDD_CURRENT(kdata.close());
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

    CHECK_EQ(m2.name(), "MDD_CURRENT");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */