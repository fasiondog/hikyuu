/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/INDEX.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_INDEX test_indicator_INDEX
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_INDEXO") {
    /** @arg 空指标 */
    Indicator result = INDEXO();
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.name(), "INDEXO");

    /** @arg 上证日线 */
    KQuery query = KQueryByDate(Datetime(20111130), Datetime(20111206));
    auto k = getKData("sh600004", query);
    REQUIRE(k.size() > 0);
    result = INDEXO(k);
    CHECK_EQ(result.name(), "INDEXO");
    CHECK_EQ(result.size(), k.size());

    auto expect_k = getKData("sh000001", query);
    Indicator expect = OPEN(expect_k);
    for (size_t i = 0, total = result.size(); i < total; ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg 上证5分钟线 */
    query = KQueryByDate(Datetime(201111300930), Datetime(201111301400), KQuery::MIN5);
    k = getKData("sh600004", query);
    REQUIRE(k.size() > 0);
    result = INDEXO(k);
    CHECK_EQ(result.name(), "INDEXO");
    CHECK_EQ(result.size(), k.size());

    expect_k = getKData("sh000001", query);
    expect = OPEN(expect_k);
    for (size_t i = 0, total = result.size(); i < total; ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg 上证周线 */
    query = KQueryByDate(Datetime(20111101), Datetime(20111201), KQuery::WEEK);
    k = getKData("sh600004", query);
    REQUIRE(k.size() > 0);
    result = INDEXO(k);
    CHECK_EQ(result.name(), "INDEXO");
    CHECK_EQ(result.size(), k.size());

    expect_k = getKData("sh000001", query);
    expect = OPEN(expect_k);
    for (size_t i = 0, total = result.size(); i < total; ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_INDEX_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/INDEX.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-5));
    Indicator x1 = INDEXC(kdata);
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
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isinf(x1[i])) {
            CHECK_UNARY(std::isinf(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
