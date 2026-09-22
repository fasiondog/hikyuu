/*
 * test_STD.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_STDEV test_indicator_STDEV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_STDEV") {
    /** @arg The normal case with n > 1 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 10);
    // BREAKING CHANGE: discard changed from 1 to data.discard()+n-1=9 (NaN while the window is not
    // full)
    CHECK_EQ(dev.discard(), 9);
    CHECK_EQ(dev.size(), 15);

    // While the window is not full (i<9) NaN is output; from i>=9 the steady value matches the old
    // implementation (within the Welford precision)
    vector<price_t> expected{0, 0,       0,       0,       0,       0,     0,      0,
                             0, 2.92309, 3.14289, 2.83039, 3.26769, 3.653, 4.00139};
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    for (size_t i = dev.discard(); i < dev.size(); i++) {
        CHECK_EQ(dev[i], doctest::Approx(expected[i]).epsilon(0.0001));
    }

    /** @arg When n = 1 */
    CHECK_THROWS_AS(STDEV(ind, 1), std::exception);

    /** @arg operator() */
    Indicator expect = STDEV(ind, 10);
    dev = STDEV(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = expect.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par Test points */
TEST_CASE("test_STDEV_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = STDEV(c, 10);
    Indicator result = STDEV(c, CVAL(c, 10));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = STDEV(c, IndParam(CVAL(c, 10)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_STDEV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STDEV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = STDEV(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

//----------------------------------------------------------------------------
// The NaN semantics test (the Welford rolling variance + the conditional recalculation when an
// outlier leaves)
//----------------------------------------------------------------------------

/** @par Test point: the rolling standard deviation with scattered NaN */
TEST_CASE("test_STDEV_nan_scattered") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(3.0);
    d.push_back(Null<price_t>());  // NaN
    d.push_back(5.0);
    d.push_back(6.0);
    d.push_back(7.0);
    d.push_back(8.0);
    d.push_back(9.0);
    d.push_back(10.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 4);
    CHECK_EQ(dev.discard(), 3);
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    // i=3, the window [1,2,3,NaN] -> vc=3, std=std(1,2,3)=1.0
    CHECK_EQ(dev[3], doctest::Approx(1.0).epsilon(0.0001));
    // i=4, the window [2,3,NaN,5] -> vc=3, std=std(2,3,5)=1.527525
    CHECK_EQ(dev[4], doctest::Approx(1.527525).epsilon(0.0001));
    // i=5, the window [3,NaN,5,6] -> vc=3, std=std(3,5,6)=1.527525
    CHECK_EQ(dev[5], doctest::Approx(1.527525).epsilon(0.0001));
    // i=6, the window [NaN,5,6,7] -> vc=3, std=std(5,6,7)=1.0 (recovered)
    CHECK_EQ(dev[6], doctest::Approx(1.0).epsilon(0.0001));
    // i=7, the window [5,6,7,8] -> vc=4, std=std(5,6,7,8)=1.290994
    CHECK_EQ(dev[7], doctest::Approx(1.290994).epsilon(0.0001));
    CHECK_EQ(dev[8], doctest::Approx(1.290994).epsilon(0.0001));
    CHECK_EQ(dev[9], doctest::Approx(1.290994).epsilon(0.0001));
}

/** @par Test point: the state recovery after passing through consecutive NaN */
TEST_CASE("test_STDEV_nan_consecutive") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(Null<price_t>());
    d.push_back(Null<price_t>());
    d.push_back(Null<price_t>());
    d.push_back(3.0);
    d.push_back(4.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // i=2, the window [1,2,NaN] -> vc=2, std=std(1,2)=0.707107
    CHECK_EQ(dev[2], doctest::Approx(0.707107).epsilon(0.0001));
    // i=3, the window [2,NaN,NaN] -> vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[3]));
    // i=4, the window [NaN,NaN,NaN] -> vc=0, std=NaN
    CHECK_UNARY(std::isnan(dev[4]));
    // i=5, the window [NaN,NaN,3] -> vc=1, std=NaN (rebuilt from 0)
    CHECK_UNARY(std::isnan(dev[5]));
    // i=6, the window [NaN,3,4] -> vc=2, std=std(3,4)=0.707107 (recovered)
    CHECK_EQ(dev[6], doctest::Approx(0.707107).epsilon(0.0001));
}

/** @par Test point: the window has a single valid value (count=1, std=NaN instead of Inf) */
TEST_CASE("test_STDEV_nan_single_valid") {
    PriceList d;
    for (int i = 0; i < 7; ++i) {
        d.push_back(Null<price_t>());
    }
    d[3] = 5.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 4);
    CHECK_EQ(dev.discard(), 3);
    // vc is always 1 and std always NaN (the sample variance divides by 0, NaN is returned safely)
    for (size_t i = dev.discard(); i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
}

/** @par Test point: a large base with a small variance (the Welford precision check) */
TEST_CASE("test_STDEV_large_base") {
    PriceList d;
    double base = 1e8;
    for (int i = 1; i <= 5; ++i) {
        d.push_back(base + i);
    }

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // The window [1e8+1,1e8+2,1e8+3] -> std=1.0 (exact, Welford removes the large base)
    CHECK_EQ(dev[2], doctest::Approx(1.0).epsilon(0.0001));
    CHECK_EQ(dev[3], doctest::Approx(1.0).epsilon(0.0001));
    CHECK_EQ(dev[4], doctest::Approx(1.0).epsilon(0.0001));
}

/** @par Test point: an outlier leaving the window triggers a recalculation */
TEST_CASE("test_STDEV_outlier_leaving_rescan") {
    PriceList d;
    d.push_back(10.0);
    d.push_back(10.0);
    d.push_back(10.0);
    d.push_back(1e6);  // The outlier
    d.push_back(2.0);
    d.push_back(4.0);
    d.push_back(4.0);
    d.push_back(4.0);
    d.push_back(5.0);
    d.push_back(5.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 5);
    CHECK_EQ(dev.discard(), 4);
    // i=8, the outlier 1e6 leaves the window, the window is [2,4,4,4,5]
    // The true std = sqrt(1.2) ~ 1.095445, the recalculation must restore it exactly
    CHECK_EQ(dev[8], doctest::Approx(1.095445).epsilon(0.00001));
    // i=9, the window [4,4,4,5,5] -> std=0.547723
    CHECK_EQ(dev[9], doctest::Approx(0.547723).epsilon(0.0001));
}

/** @par Test point: the minimum window n=2 */
TEST_CASE("test_STDEV_n2") {
    PriceList d;
    d.push_back(3.0);
    d.push_back(Null<price_t>());
    d.push_back(5.0);
    d.push_back(7.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 2);
    CHECK_EQ(dev.discard(), 1);
    // i=1, the window [3,NaN] -> vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[1]));
    // i=2, the window [NaN,5] -> vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[2]));
    // i=3, the window [5,7] -> vc=2, std=std(5,7)=1.414214
    CHECK_EQ(dev[3], doctest::Approx(1.414214).epsilon(0.0001));
}

/** @par Test point: an all-NaN sequence neither loops forever nor throws */
TEST_CASE("test_STDEV_all_nan") {
    PriceList d;
    for (int i = 0; i < 4; ++i) {
        d.push_back(Null<price_t>());
    }
    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.size(), 4);
    for (size_t i = 0; i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
}

/** @par Test point: the upstream discard propagation */
TEST_CASE("test_STDEV_upstream_discard") {
    PriceList d;
    for (int i = 0; i < 10; ++i) {
        d.push_back(i + 1);
    }
    for (int i = 0; i < 4; ++i) {
        d[i] = Null<price_t>();
    }
    Indicator ind = PRICELIST(d);
    ind.setDiscard(4);

    Indicator dev = STDEV(ind, 3);
    // m_discard = 4 + 3 - 1 = 6
    CHECK_EQ(dev.discard(), 6);
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    // i=6, the window [5,6,7] -> std=1.0
    CHECK_EQ(dev[6], doctest::Approx(1.0).epsilon(0.0001));
}

/** @par Test point: the _dyn dynamic path is equivalent to the static n (with NaN) */
TEST_CASE("test_STDEV_dyn_nan_equivalence") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(3.0);
    d.push_back(Null<price_t>());
    d.push_back(5.0);
    d.push_back(6.0);
    d.push_back(7.0);
    d.push_back(8.0);

    Indicator ind = PRICELIST(d);
    Indicator expect = STDEV(ind, 4);
    Indicator result = STDEV(ind, CVAL(ind, 4));
    CHECK_EQ(expect.size(), result.size());
    // The _dyn path has the window-not-full guard, so its discard matches the static one
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(result[i]));
        } else {
            CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.0001));
        }
    }
}

/** @par Test point: the n=1 parameter is intercepted (the sample standard deviation is undefined,
 * an exception is thrown instead of Inf/a crash) */
TEST_CASE("test_STDEV_n1_rejected") {
    PriceList d;
    for (int i = 0; i < 5; ++i) {
        d.push_back(i + 1);
    }
    Indicator ind = PRICELIST(d);
    // n=1: the sample variance divides by N-1=0 and _checkParam intercepts it (HKU_ASSERT(n==0 ||
    // n>=2))
    CHECK_THROWS_AS(STDEV(ind, 1), std::exception);
}

/** @par Test point: the zero variance of a constant sequence (the floating point noise defense) */
TEST_CASE("test_STDEV_zero_variance") {
    PriceList d;
    for (int i = 0; i < 5; ++i) {
        d.push_back(3.14);
    }
    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // The variance of a constant sequence is 0; the floating point noise may make M2 slightly
    // negative, the max(0,...) defense ensures 0.0 instead of NaN
    for (size_t i = dev.discard(); i < dev.size(); ++i) {
        CHECK_EQ(dev[i], doctest::Approx(0.0).epsilon(0.0001));
    }
}

/** @} */
