/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ISNA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ISNA test_indicator_ISNA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ISNA") {
    price_t nan = Null<price_t>();

    /** @arg ignore_discard 为 false */
    Indicator data = PRICELIST(PriceList{nan, nan, 1, nan, 3}, 2);
    Indicator result = ISNA(data, false);
    CHECK_EQ(result.name(), "ISNA");
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.size(), data.size());
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    auto expect = PriceList{nan, nan, 0., 1., 0.};
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg ignore_discard 为 true */
    result = ISNA(data, true);
    CHECK_EQ(result.name(), "ISNA");
    CHECK_EQ(result.discard(), 0);
    expect = PriceList{1., 1., 0., 1., 0.};
    CHECK_EQ(result.size(), data.size());
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ISNA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ISNA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-5));
    Indicator x1 = ISNA(CLOSE(kdata));
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
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
