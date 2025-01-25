/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
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

/** @par 检测点 */
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

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_COST_benchmark") {
    Stock stock = getStock("sz000001");
    KData kdata = stock.getKData(KQuery(0));

    int cycle = 1000;  // 测试循环次数

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

/** @par 检测点 */
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