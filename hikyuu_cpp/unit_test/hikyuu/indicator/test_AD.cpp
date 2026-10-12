/*
 * test_AD.cpp
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <hikyuu/KDataPrivatedBufferImp.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AD.h>

using namespace hku;

/**
 * @defgroup test_indicator_AD test_indicator_AD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AD") {
    Stock stk = getStock("SH600004");
    KQuery query = KQueryByIndex(-10);
    KData k = stk.getKData(query);

    Indicator ad = AD(k);
    CHECK_EQ(ad.name(), "AD");
    CHECK_EQ(ad.size(), k.size());
    CHECK_EQ(ad.discard(), 0);
    CHECK_EQ(ad[0], 0);
    CHECK_EQ(ad[1], doctest::Approx(458.65).epsilon(0.1));
    CHECK_EQ(ad[2], doctest::Approx(63.99).epsilon(0.1));
    CHECK_EQ(ad[5], doctest::Approx(30.77).epsilon(0.1));
}

/** @par Test points: a NaN high/low bar is skipped and does not poison the AD accumulator */
TEST_CASE("test_AD_nan_middle") {
    Stock stock = StockManager::instance().getStock("sh000001");
    REQUIRE_FALSE(stock.isNull());

    // Bars 0/1/2 valid; bar 3 has NaN high; bars 4/5 valid again. The old impl read
    // `if (tmp != 0.0)` and let a NaN tmp through (NaN != 0 is true) -> ad became NaN forever.
    const price_t nan = std::numeric_limits<price_t>::quiet_NaN();
    Datetime date(2010, 1, 1);
    KRecordList records;
    for (size_t i = 0; i < 6; ++i) {
        price_t high = (i == 3) ? nan : 12.0;
        records.push_back(KRecord(date + Days(i), 10.0, high, 9.0, 11.0, 1000.0, 100.0));
    }
    KData kdata =
      KData(make_shared<KDataPrivatedBufferImp>(stock, KQuery(0, records.size()), records));

    Indicator ad = AD(kdata);
    REQUIRE_EQ(ad.size(), 6);

    /** @arg every output bar stays finite; the NaN bar is skipped, accumulator does not poison */
    for (size_t i = 0; i < ad.size(); ++i) {
        CHECK_UNARY(std::isfinite(ad[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_AD_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_AD_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = AD();
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
TEST_CASE("test_AD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AD.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = AD(k);
    x1.setContext(k);

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

    CHECK_EQ(x2.name(), "AD");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
