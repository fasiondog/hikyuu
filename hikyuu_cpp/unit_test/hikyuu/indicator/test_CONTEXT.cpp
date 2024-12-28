/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CONTEXT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CONTEXT test_indicator_CONTEXT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CONTEXT") {
    auto k1 = getKData("sz000001", KQuery(-30));
    auto k2 = getKData("sz000002", KQuery(-30));
    price_t nan = Null<price_t>();

    /** @arg 空对象 */
    auto cnt = CONTEXT();
    CHECK_UNARY(cnt.empty());
    cnt = cnt(k1);
    CHECK_UNARY(cnt.empty());

    /** @arg 时间无关序列 */
    cnt = CONTEXT(PRICELIST(PriceList{nan, nan, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 2));
    CHECK_EQ(cnt.size(), 12);
    CHECK_EQ(cnt.discard(), 2);
    auto result = cnt(k1);
    CHECK_EQ(result.size(), k1.size());
    CHECK_EQ(result.discard(), 20);
    for (size_t i = 0, len = result.discard(); i < len; i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(), total = result.size(), j = 1; i < total; i++, j++) {
        CHECK_EQ(result[i], j);
    }

    /** @arg 无上下文的，时间序列 */
    auto dates = getStock("sz000001").getDatetimeList(KQuery(-12));
    cnt = CONTEXT(PRICELIST(PriceList{nan, nan, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, dates, 2));
    CHECK_EQ(cnt.size(), 12);
    CHECK_EQ(cnt.discard(), 2);
    result = cnt(k1);
    CHECK_EQ(result.size(), k1.size());
    CHECK_EQ(result.discard(), 20);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CONTEXT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CONTEXT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = CONTEXT(CLOSE(kdata));
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
