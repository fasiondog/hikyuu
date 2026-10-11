/*
 * test_ABS.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ZSCORE.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ZSCORE test_indicator_ZSCORE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ZSCORE") {
    /** @arg There is only one valid record */
    PriceList a{0.3};
    Indicator data = PRICELIST(a);
    REQUIRE(data.size() == 1);
    Indicator result = ZSCORE(data);
    CHECK_EQ(result.name(), "ZSCORE");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg The input nsigma < 0 */
    KData k = getKData("SH000001", KQuery(-5));
    CHECK_THROWS_AS(ZSCORE(k.close(), true, -0.5), std::exception);

    /** @arg The normal calculation without removing the outliers */
    result = ZSCORE(k.close());
    CHECK_EQ(result.name(), "ZSCORE");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.discard(), 0);

    PriceList expect{-0.573824, 1.52671, 0.497154, -0.581095, -0.868942};
    for (size_t i = result.discard(), total = result.size(); i < total; i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    /** @arg Filter the outliers, not recursively */
    k = getKData("SH000001", KQuery(3600, 4000));
    Indicator c = k.close();
    result = ZSCORE(c, true, 3.0, false);
    expect = {-0.87128, -0.90351, -0.87397, -0.87383};
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    /** @arg Filter the outliers recursively */
    k = getKData("SH000001", KQuery(3600, 4000));
    c = k.close();
    auto result2 = ZSCORE(c, true, 3.0, true);
    expect = {-0.87128, -0.90354, -0.87399, -0.87383};
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result2[i], doctest::Approx(expect[i]));
    }
    CHECK_EQ(result[16], doctest::Approx(-0.95994));
    CHECK_EQ(result2[16], doctest::Approx(-0.95996));

    /** @arg A constant series makes sigma=0, z-score is undefined -> all NaN (ST-018) */
    PriceList flat{5.0, 5.0, 5.0, 5.0, 5.0};
    Indicator zflat = ZSCORE(PRICELIST(flat));
    CHECK_EQ(zflat.size(), flat.size());
    CHECK_EQ(zflat.discard(), flat.size());
    for (size_t i = 0; i < zflat.size(); ++i) {
        CHECK_UNARY(std::isnan(zflat[i]));
    }
}

/** @par Test points: ST-034 — non-finite inputs (NaN or ±Inf) are excluded from mean/sigma */
TEST_CASE("test_ZSCORE_inf_input_excluded") {
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator data = PRICELIST({1.0, 2.0, inf_v, 4.0, 5.0});
    Indicator result = ZSCORE(data);
    // Inf slot is excluded from mean/sigma; its own output stays Null (NaN)
    CHECK_UNARY(std::isnan(result[2]));
    // Every output must be non-Inf (either NaN or finite) — AGENTS.md L247 contract
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_UNARY(std::isnan(result[i]) || std::isfinite(result[i]));
    }
    // Finite outputs are standardized over the 4 valid points (mean=3, sample sigma=sqrt(10/3))
    CHECK_EQ(result[0], doctest::Approx((1.0 - 3.0) / std::sqrt(10.0 / 3.0)));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ZSCORE_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_ZSCORE_benchmark, cycle,
                           fmt::format(fmt::runtime("data len: {}"), c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = ZSCORE();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ZSCORE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ZSCORE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ZSCORE(CLOSE(kdata));
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
