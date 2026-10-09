/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
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

/** @par Test points */
TEST_CASE("test_INDEXO") {
    /** @arg An empty indicator */
    Indicator result = INDEXO();
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.name(), "INDEXO");

    /** @arg The Shanghai Composite daily line */
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

    /** @arg The Shanghai Composite 5-minute line */
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

    /** @arg The Shanghai Composite weekly line */
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

/**
 * @par Test points
 * The index code of the current security is a result of each calculation, not a permanent
 * parameter: writing it back with setParam leaks the previous security's index into every later
 * calculation which falls back to the parameter.
 *
 * Note: the companion fix declares INDEX as a context dependent indicator (m_need_context), which
 * keeps a compiled factor plan from reusing one template node across securities; that flag has no
 * public accessor to assert on.
 */
TEST_CASE("test_INDEX_market_code_not_permanent") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_FALSE(stock.isNull());
    KData k = stock.getKData(KQuery(0, 20));
    CHECK_EQ(k.size(), 20);

    // The daily data of the Shanghai Composite Index is a leaf of the test dataset, so the given
    // code must stay unused by the calculation of an A share of the SH market
    Indicator ind = INDEXC();
    ind.getImp()->setParam<string>("market_code", "SH000001");
    ind.setContext(k);

    /** @arg the calculation keeps the parameter as it was given */
    CHECK_EQ(ind.getImp()->getParam<string>("market_code"), string("SH000001"));

    // A user given code must survive a calculation of a security whose own index is derived
    Indicator custom = INDEXC();
    custom.getImp()->setParam<string>("market_code", "SZ399001");
    custom.setContext(k);

    /** @arg a calculation of a derived security does not drop the given code either */
    CHECK_EQ(custom.getImp()->getParam<string>("market_code"), string("SZ399001"));

    /** @arg the values still follow the index derived for the current security */
    Indicator expect = INDEXC();
    expect.setContext(k);
    CHECK_EQ(custom.size(), expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(custom[i], doctest::Approx(expect[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
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
