/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/COST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/DMA.h>
#include <hikyuu/indicator/crt/HSL.h>

/**
 * @defgroup test_indicator_COST test_indicator_COST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_COST") {
    auto k = getKData("sz000001", KQueryByIndex(-50));
    auto result = COST(k, 10);
    auto expect = DMA((CLOSE(k) + (HIGH(k) - LOW(k)) * 0.1), HSL(k));

    CHECK_EQ(result.name(), "COST");
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }
}

/**
 * @par Test points
 * The cost of the bars from the first ex-rights record on must be a running recurrence, not null.
 *
 * Background: the recurrence read the previous bar of the buffer, which is still null when the
 * first ex-rights record of the stock is later than the first bar of the context, so one null
 * poisoned every later bar and the whole result ended up discarded.
 */
TEST_CASE("test_COST_first_ex_right_inside_context") {
    Stock stock = getStock("sz000001");
    REQUIRE_FALSE(stock.isNull());
    // The oldest bars of the stock, with its first ex-rights record falling inside the window
    KData k = stock.getKData(KQuery(0, 120));
    REQUIRE_EQ(k.size(), 120);

    Indicator cost = COST(k);

    /** @arg the first ex-rights record of the window is after its first bar, which is the very
     *  premise of the defect: the recurrence has to seed itself */
    CHECK_UNARY(cost.discard() > 0);

    /** @arg the result keeps the context length and holds valid values */
    CHECK_EQ(cost.size(), 120);
    CHECK_UNARY(cost.discard() < cost.size());
    /** @arg nothing is null from the discard on, and the costs stay positive */
    for (size_t i = cost.discard(); i < cost.size(); ++i) {
        CHECK_UNARY(!std::isnan(cost[i]));
        CHECK_UNARY(cost[i] > 0.0);
    }
    /** @arg the last bar, the tail of the whole recurrence, holds a value */
    CHECK_UNARY(!std::isnan(cost[cost.size() - 1]));
}

/**
 * @par Test points
 * A window which spans several ex-rights records computes those bars inside the ex-rights loop, so
 * the seeding there is exercised as well; the whole result must match the DMA definition.
 */
TEST_CASE("test_COST_multiple_ex_rights_in_context") {
    Stock stock = getStock("sz000001");
    REQUIRE_FALSE(stock.isNull());
    KData k = stock.getKData(KQuery(0, 600));
    REQUIRE_EQ(k.size(), 600);

    size_t record_num = 0;
    for (const auto& sw : stock.getWeight(Datetime::min(), k.back().datetime + Days(1))) {
        if (sw.freeCount() > 0) {
            ++record_num;
        }
    }
    /** @arg the window really spans several usable ex-rights records */
    REQUIRE(record_num > 1);

    auto result = COST(k, 10);
    auto expect = DMA((CLOSE(k) + (HIGH(k) - LOW(k)) * 0.1), HSL(k));

    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_COST_benchmark") {
    Stock stock = getStock("sz000001");
    KData kdata = stock.getKData(KQuery(0));

    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_COST_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = COST(10);
            Indicator result = ind(kdata);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_COST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COST.xml";

    Stock stock = sm.getStock("sz000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = COST(kdata, 10);
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
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */