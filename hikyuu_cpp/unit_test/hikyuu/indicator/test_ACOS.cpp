/*
 * test_ACOS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ACOS.h>
#include <hikyuu/indicator/crt/COS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ACOS test_indicator_ACOS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ACOS") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i / 9.);
    }

    Indicator data = PRICELIST(a);

    result = ACOS(data);
    CHECK_EQ(result.name(), "ACOS");
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], std::acos(data[i]));
    }

    result = ACOS(-0.1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(std::acos(-0.1)));

    result = ACOS(-1.1);
    CHECK_UNARY(std::isnan(result[0]));

    result = ACOS(2.1);
    CHECK_UNARY(std::isnan(result[0]));
}

/**
 * @par Test points
 * Incremental calculate must match a full calculation.
 *
 * Background: _increment_calculate looped from m_discard instead of start_pos. The framework resets
 * m_discard to 0 before the incremental call, so each incremental run recomputed the whole series
 * from bar 0 (a pure performance defect; results stayed correct). ACOS(COS(CLOSE())) keeps the
 * input within [-1, 1] so the compared values are finite rather than an all-NaN trivial pass.
 */
TEST_CASE("test_ACOS_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));
    CHECK_EQ(k_full.size(), 20);

    Indicator expect = ACOS(COS(CLOSE()));
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = ACOS(COS(CLOSE()));
        got.setContext(stock.getKData(KQuery(0, history)));  // cache m_old_context
        got.setContext(k_full);                              // extended at the tail
        CHECK_EQ(got.size(), expect.size());
        CHECK_EQ(got.discard(), expect.discard());
        for (size_t i = 0; i < expect.size(); ++i) {
            double a = expect[i];
            double b = got[i];
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK_EQ(b, doctest::Approx(a).epsilon(0.0001));
        }
    };

    /** @arg The old context holds half the bars, tail extended incrementally */
    check_with_history(10);

    /** @arg The old context holds most bars, a small tail extension */
    check_with_history(15);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ACOS_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_ACOS_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = ACOS();
            Indicator result = ind(c);
            DO_NOT_OPTIMIZE(result);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ACOS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ACOS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ACOS(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "ACOS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
