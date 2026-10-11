/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/device.h>
#include <hikyuu/indicator/imp/ITsRank.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_TS_RANK test_indicator_TS_RANK
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test point: the basic functionality test of TS_RANK */
TEST_CASE("test_TS_RANK_base") {
    Indicator ind = PRICELIST({5.0, 3.0, 8.0, 3.0, 2.0, 9.0, 1.0, 7.0, 4.0, 6.0}, 0);

    Indicator ret = TS_RANK(ind, 5);

    CHECK_EQ(ret.name(), "TS_RANK");
    CHECK_EQ(ret.size(), 10);
    CHECK_EQ(ret.discard(), 4);

    price_t nan = Null<price_t>();
    Indicator expect = PRICELIST({nan, nan, nan, nan, 0.2, 1.0, 0.2, 0.8, 0.6, 0.6}, 4);
    check_indicator(ret, expect);
}

/** @par Test point: the behavior of TS_RANK with the parameter 1 */
TEST_CASE("test_TS_RANK_n1") {
    Indicator ind = PRICELIST({1.0, 2.0, 3.0, 4.0, 5.0});

    Indicator ret = TS_RANK(ind, 1);

    CHECK_EQ(ret.size(), 5);
    CHECK_EQ(ret.discard(), 0);

    Indicator expect = PRICELIST({1.0, 1.0, 1.0, 1.0, 1.0});
    check_indicator(ret, expect);
}

/** @par Test point: TS_RANK handling the NaN values */
TEST_CASE("test_TS_RANK_with_nan") {
    price_t nan = Null<price_t>();
    Indicator ind = PRICELIST({nan, 3.0, 5.0, 4.0, 6.0, 2.0}, 1);

    Indicator ret = TS_RANK(ind, 3);

    CHECK_EQ(ret.size(), 6);
    CHECK_EQ(ret.discard(), 3);

    /** @arg ST-019: current is NaN inside the loop range -> dst is NaN, not 0 (fake lowest rank) */
    Indicator ind2 = PRICELIST({3.0, 5.0, nan, 4.0, 6.0, 2.0}, 0);
    Indicator ret2 = TS_RANK(ind2, 3);
    CHECK_EQ(ret2.size(), 6);
    CHECK_UNARY(std::isnan(ret2[0]));
    CHECK_UNARY(std::isnan(ret2[1]));
    CHECK_UNARY(std::isnan(ret2[2]));  // src[2] NaN in the valid region
    // ST-026: window NaN slots are excluded from both numerator and denominator.
    // i=3 window [1..3]={5.0, nan, 4.0}: valid=2 (5.0, 4.0), count_le(current=4.0)=1 -> 1/2
    CHECK_EQ(ret2[3], doctest::Approx(1.0 / 2.0));
    // i=4 window [2..4]={nan, 4.0, 6.0}: valid=2, count_le(current=6.0)=2 -> 1.0
    CHECK_EQ(ret2[4], doctest::Approx(1.0));
    // i=5 window [3..5]={4.0, 6.0, 2.0}: valid=3, count_le(current=2.0)=1 -> 1/3
    CHECK_EQ(ret2[5], doctest::Approx(1.0 / 3.0));
}

/** @par Test point: ST-026 window entirely NaN except current -> valid=1, rank=1.0 */
TEST_CASE("test_TS_RANK_window_mostly_nan") {
    price_t nan = Null<price_t>();
    Indicator ind = PRICELIST({nan, nan, nan, 5.0}, 0);
    Indicator ret = TS_RANK(ind, 4);
    CHECK_EQ(ret.size(), 4);
    // m_discard = 0 + 4 - 1 = 3, only i=3 is in the valid region
    // window [0..3] = {nan, nan, nan, 5.0}: valid=1 (5.0 itself), count_le=1 -> 1.0
    CHECK_EQ(ret[3], 1.0);
}

/** @par Test point: TS_RANK handling the empty data */
TEST_CASE("test_TS_RANK_empty") {
    Indicator ind;

    Indicator ret = TS_RANK(ind, 5);

    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);
}

/** @par Test point: the basic functionality of the TS_RANK incremental calculation */
TEST_CASE("test_TS_RANK_increment_base") {
    Stock stk = getStock("sh600000");
    KData k1 = stk.getKData(KQuery(100, 200));
    KData k2 = stk.getKData(KQuery(100, 300));

    auto ts_rank = TS_RANK(CLOSE(), 5);
    auto result_full = ts_rank(k2);
    auto result_inc = ts_rank(k1);

    result_inc.setContext(k2);

    check_indicator(result_inc, result_full);
}

/** @par Test points: ±Inf in the window is excluded from both numerator and denominator */
TEST_CASE("test_TS_RANK_inf_input") {
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator data = PRICELIST({1.0, 2.0, inf_v, 4.0, 5.0});
    Indicator r = TS_RANK(data, 3);
    for (size_t i = 0; i < r.size(); i++) {
        CHECK_UNARY(std::isnan(r[i]) || (r[i] >= 0.0 && r[i] <= 1.0));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test point: the TS_RANK serialization */
TEST_CASE("test_TS_RANK_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TS_RANK.xml";

    Indicator ind = PRICELIST({5.0, 3.0, 8.0, 3.0, 2.0, 9.0, 1.0, 7.0, 4.0, 6.0}, 0);
    Indicator x1 = TS_RANK(ind, 5);
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

    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */