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
#include <hikyuu/indicator/crt/ATR.h>
#include <hikyuu/indicator/crt/TR.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/DISCARD.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_ATR test_indicator_ATR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ATR") {
    auto k = getKData("sh000001", KQuery(-30));

    auto atr = ATR(k, 10);
    CHECK_EQ(atr.name(), "ATR");
    CHECK_EQ(atr.size(), k.size());
    CHECK_EQ(atr.discard(), 11);

    auto expect = DISCARD(MA(TR(), 10), 11)(k);
    for (size_t i = atr.discard(); i < atr.size(); ++i) {
        CHECK_EQ(atr[i], doctest::Approx(expect[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ATR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_ATR_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = ATR();
            Indicator result = ind(kdata);
        }
    }
}
#endif

/**
 * @par Test points
 * Incremental calculate must match a full calculation, including a first context that was fully
 * discarded.
 *
 * Background: IAtr declared incremental support but not min_increment_start(), so the framework
 * could only raise start_pos with the previous m_discard. When the first context was shorter than
 * n+1, that m_discard degraded to its own size and left start_pos < n: the TR seed loop was
 * skipped, dst[start_pos-1] read a discarded NaN and buf[i-n] read out of bounds, turning the
 * whole result into NaN.
 */
TEST_CASE("test_ATR_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = ATR(14);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = ATR(14);
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

    /** @arg The first context was fully discarded, so start_pos fell below n */
    check_with_history(5);

    /** @arg The first context already had valid results, the usual incremental path */
    check_with_history(20);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ATR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ATR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ATR(kdata);
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

    CHECK_EQ(x2.name(), "ATR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
