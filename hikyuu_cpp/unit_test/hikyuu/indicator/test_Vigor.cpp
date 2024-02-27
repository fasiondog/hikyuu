/*
 * test_Vigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/VIGOR.h>

using namespace hku;

/**
 * @defgroup test_indicator_VIGOR test_indicator_VIGOR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_VIGOR") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata;
    Indicator vigor;

    /** @arg kdata为空时 */
    vigor = VIGOR(kdata, 1);
    CHECK_EQ(vigor.size(), 0);
    CHECK_EQ(vigor.empty(), true);

    /** @arg 正常情况 */
    KQuery query = KQuery(0, 10);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 10);
    vigor = VIGOR(kdata, 1);
    CHECK_EQ(vigor.discard(), 1);
    CHECK_EQ(vigor.size(), 10);
    CHECK_UNARY_FALSE(vigor.empty());
    CHECK_UNARY(std::isnan(vigor[0]));
    CHECK_EQ(vigor[1], doctest::Approx(-11761.36));

    vigor = VIGOR(kdata, 2);
    CHECK_EQ(vigor.discard(), 1);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_VIGOR_benchemark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_VIGOR_benchemark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = VIGOR();
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
TEST_CASE("test_VIGOR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/VIGOR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = VIGOR(kdata, 10);
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

/** @} */
