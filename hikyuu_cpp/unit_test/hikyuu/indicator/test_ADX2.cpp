/*
 * test_ADX2.cpp
 *
 *  Created on: 2026-07-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ADX2.h>

using namespace hku;

/**
 * @defgroup test_indicator_ADX2 test_indicator_ADX2
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ADX2") {
    auto k = getKData("sh000001", KQuery(-100));

    auto adx2 = ADX2(k, 14);
    CHECK_EQ(adx2.name(), "ADX2");
    CHECK_EQ(adx2.size(), k.size());
    CHECK_EQ(adx2.getResultNumber(), 3);
}

/** @par 检测点：验证具体计算结果 */
TEST_CASE("test_ADX2_calculation") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-200));
    auto adx2 = ADX2(k, 14);

    CHECK_EQ(adx2.discard(), 27);
    CHECK_EQ(adx2.size(), 200);

    size_t pos = 100;
    Indicator::value_t adx2_val = adx2.get(pos, 0);
    Indicator::value_t pdi_val = adx2.get(pos, 1);
    Indicator::value_t mdi_val = adx2.get(pos, 2);

    CHECK_FALSE(std::isnan(adx2_val));
    CHECK_FALSE(std::isnan(pdi_val));
    CHECK_FALSE(std::isnan(mdi_val));

    CHECK(adx2_val >= 0);
    CHECK(adx2_val <= 100);
    CHECK(pdi_val >= 0);
    CHECK(pdi_val <= 100);
    CHECK(mdi_val >= 0);
    CHECK(mdi_val <= 100);

    Indicator::value_t smooth_factor = 2.0 / 15.0;
    Indicator::value_t prev_factor = 1.0 - smooth_factor;
    for (size_t i = adx2.discard() + 1; i < adx2.size(); ++i) {
        Indicator::value_t dx_val = adx2.get(i, 1) + adx2.get(i, 2);
        if (dx_val > 0) {
            Indicator::value_t expected_dx =
              100.0 * std::abs(adx2.get(i, 1) - adx2.get(i, 2)) / dx_val;
            Indicator::value_t expected_adx2 =
              adx2.get(i - 1, 0) * prev_factor + expected_dx * smooth_factor;
            CHECK(adx2.get(i, 0) == doctest::Approx(expected_adx2).epsilon(0.01));
        }
    }
}

/** @par 检测点：验证结果集顺序 */
TEST_CASE("test_ADX2_result_order") {
    auto k = getKData("sh000001", KQuery(-100));

    auto adx2 = ADX2(k, 14);

    size_t last_pos = adx2.size() - 1;
    Indicator::value_t adx2_val = adx2.get(last_pos, 0);
    Indicator::value_t pdi_val = adx2.get(last_pos, 1);
    Indicator::value_t mdi_val = adx2.get(last_pos, 2);

    CHECK_FALSE(std::isnan(adx2_val));
    CHECK_FALSE(std::isnan(pdi_val));
    CHECK_FALSE(std::isnan(mdi_val));
}

/** @par 检测点：ADX与ADX2对比 */
TEST_CASE("test_ADX_vs_ADX2") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-200));

    auto adx = ADX(k, 14);
    auto adx2 = ADX2(k, 14);

    CHECK_EQ(adx.size(), adx2.size());
    CHECK_EQ(adx.getResultNumber(), adx2.getResultNumber());

    CHECK_EQ(adx.discard(), adx2.discard());
    CHECK_EQ(adx.discard(), 27);

    size_t last_pos = adx.size() - 1;
    Indicator::value_t adx_val = adx.get(last_pos, 0);
    Indicator::value_t adx2_val = adx2.get(last_pos, 0);

    CHECK_FALSE(std::isnan(adx_val));
    CHECK_FALSE(std::isnan(adx2_val));

    CHECK_NE(adx_val, adx2_val);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ADX2_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_ADX2_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = ADX2(14);
            Indicator result = ind(kdata);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ADX2_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ADX2.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));
    Indicator x1 = ADX2(kdata, 14);

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

    CHECK_EQ(x2.name(), "ADX2");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());

    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        for (size_t j = 0; j < 3; ++j) {
            CHECK_EQ(x1.get(i, j), doctest::Approx(x2.get(i, j)));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */