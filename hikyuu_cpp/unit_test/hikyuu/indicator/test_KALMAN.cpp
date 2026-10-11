/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KALMAN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_KALMAN test_indicator_KALMAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_KALMAN") {
    Indicator src = PRICELIST(PriceList{1.0, 2.0, 3.0, 4.0, 5.0});

    /** @arg An empty indicator */
    Indicator ret = KALMAN()(Indicator());
    CHECK_UNARY(ret.empty());
    CHECK_EQ(ret.name(), "KALMAN");

    /** @arg Default q/r: output size matches, first value seeds from src[discard], later values
     * smooth */
    ret = KALMAN(src);
    CHECK_EQ(ret.name(), "KALMAN");
    CHECK_EQ(ret.size(), src.size());
    CHECK_EQ(ret.discard(), 0);
    CHECK_UNARY(!std::isnan(ret[0]));
    for (size_t i = 1; i < ret.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret[i]));
    }

    /** @arg q <= 0 or r <= 0 is rejected (ST-020) */
    CHECK_THROWS_AS(KALMAN(0.0, 0.1), std::exception);
    CHECK_THROWS_AS(KALMAN(-0.01, 0.1), std::exception);
    CHECK_THROWS_AS(KALMAN(0.01, 0.0), std::exception);
    CHECK_THROWS_AS(KALMAN(0.01, -0.1), std::exception);
}

/** @par Test points: NaN input holds state (predict-only), no permanent NaN contamination */
TEST_CASE("test_KALMAN_nan_hold") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, nan, 4.0, 5.0});
    Indicator ret = KALMAN(0.01, 0.1)(data);
    CHECK_EQ(ret.size(), 5);
    /** @arg ret[2] holds ret[1] when src[2] is NaN */
    CHECK_EQ(ret[2], ret[1]);
    /** @arg all outputs remain finite after an interior NaN */
    for (size_t i = 0; i < ret.size(); i++) {
        CHECK_UNARY(!std::isnan(ret[i]));
    }
    /** @arg subsequent valid observation still moves the estimate toward it */
    CHECK_UNARY(ret[3] > ret[2]);

    /** @arg Inf input is also held (AGENTS.md contract: no Inf poisoning downstream) */
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator data2 = PRICELIST({1.0, 2.0, inf_v, 4.0});
    Indicator ret2 = KALMAN(0.01, 0.1)(data2);
    for (size_t i = 0; i < ret2.size(); i++) {
        CHECK_UNARY(std::isfinite(ret2[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_KALMAN_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/KALMAN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = KALMAN(CLOSE(kdata), 0.01, 0.1);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
